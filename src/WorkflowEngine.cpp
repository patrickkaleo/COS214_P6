
#include "WorkflowEngine.h"
#include "WorkflowInstance.h"
#include "WorkflowDefinition.h"
// #include "f.h"
#include "InstanceData.h"
#include "WorkItemAction.h"
#include "WorkflowEventListener.h"
#include "EventDispatcher.h"
#include "DependencyMediator.h"
#include "DependencyRule.h"
#include "DepthFirstWorkItemIterator.h"
#include "AuditDecorator.h"
#include "PriorityDecorator.h"
#include "SubProcess.h"
#include "Task.h"

#include <string>
#include <vector>

// Constructor
WorkflowEngine::WorkflowEngine()
    : nextInstanceNumber(1)
{
}

// Destructor
WorkflowEngine::~WorkflowEngine()
{
  // Delete commands first because they reference instances.
  for (WorkItemAction *action : actionHistory)
  {
    delete action;
  }

  // Delete running instances
  for (auto &entry : instances)
  {
    delete entry.second;
  }

  // Delete registered definitions
  for (auto &entry : definitions)
  {
    delete entry.second;
  }

  // Detach and clean up listeners
  for (WorkflowEventListener *listener : listeners)
  {
    EventDispatcher::getInstance().detachListener(listener);
    delete listener;
  }
}

// Register a workflow definition
std::string WorkflowEngine::registerDefinition(
    WorkflowDefinition *definition)
{
  if (definition == nullptr)
  {
    return "";
  }

  std::string id = definition->getId();

  // Prevent empty or duplicate IDs
  if (id.empty() ||
      definitions.find(id) != definitions.end())
  {
    delete definition;
    return "";
  }

  // Engine takes ownership of definition
  definitions[id] = definition;

  return id;
}

// Create an independent workflow instance
WorkflowInstance *WorkflowEngine::createInstance(
    const std::string &definitionId,
    const InstanceData &data)
{
  auto found = definitions.find(definitionId);

  if (found == definitions.end())
  {
    return nullptr;
  }

  WorkflowDefinition *definition = found->second;

  // Prototype: every instance runs its own copy of the definition's tree
  WorkItem *tree = definition->instantiateTree();
  SubProcess *root = dynamic_cast<SubProcess *>(tree);

  if (root == nullptr)
  {
    delete tree;
    return nullptr;
  }

  // Apply the definition's decorators to this instance's copy of the tree
  for (const ItemDecoration &decoration : definition->getDecorations())
  {
    WorkItem *item = root->find(decoration.itemId);

    if (item == nullptr || item == root)
    {
      continue;
    }

    WorkItem *wrapper = nullptr;
    if (decoration.kind == ItemDecoration::PRIORITY)
    {
      // Priority lives on the decorator; escalation level counts real escalations
      item->setEscalationLevel(0);
      wrapper = new PriorityDecorator(item, decoration.level);
    }
    else
    {
      wrapper = new AuditDecorator(item, &attemptLog);
    }

    if (!item->getParent()->decorateChild(item, wrapper))
    {
      // Not reachable for a tree built by add(); don't let the wrapper delete a live child
      delete tree;
      return nullptr;
    }
  }

  // Each instance gets its own mediator with its own copy of the rules
  std::vector<DependencyRule> rules;
  for (DependencyRule *rule : definition->getRules())
  {
    rules.push_back(*rule);
  }

  DependencyMediator *mediator = new DependencyMediator(rules);
  mediator->registerTree(*root);

  // Generate unique instance ID
  std::string id =
      "instance-" + std::to_string(nextInstanceNumber++);

  // Create instance from registered definition
  WorkflowInstance *instance =
      new WorkflowInstance(id, definition, root, mediator, data, &directory);

  // Items route on this instance's data and report events under its id
  root->setRoutingContext(&instance->getData(), &directory);

  DepthFirstWorkItemIterator items(root);
  for (items.first(); !items.isDone(); items.next())
  {
    items.current()->setInstanceId(id);

    Task *task = dynamic_cast<Task *>(items.current()->undecorated());
    if (task != nullptr)
    {
      task->setServiceRegistry(&services);
    }
  }

  // Engine takes ownership
  instances[id] = instance;

  return instance;
}

// Retrieve a workflow instance
WorkflowInstance *WorkflowEngine::getInstance(
    const std::string &instanceId)
{
  auto found = instances.find(instanceId);

  if (found == instances.end())
  {
    return nullptr;
  }

  return found->second;
}

// Get participant directory
ParticipantDirectory &WorkflowEngine::getDirectory()
{
  return directory;
}

// Get external service registry
ServiceRegistry &WorkflowEngine::getServiceRegistry()
{
  return services;
}

// Get the attempts recorded by AuditDecorators
const AuditLogger &WorkflowEngine::getAttemptLog() const
{
  return attemptLog;
}

// Register Observer listener
void WorkflowEngine::addListener(
    WorkflowEventListener *listener)
{
  if (listener == nullptr)
  {
    return;
  }

  // Prevent registering the same pointer twice
  for (WorkflowEventListener *existing : listeners)
  {
    if (existing == listener)
    {
      return;
    }
  }

  listeners.push_back(listener);

  EventDispatcher::getInstance().attachListener(listener);
}

// Execute a Command
bool WorkflowEngine::executeAction(
    WorkItemAction *action)
{
  if (action == nullptr)
  {
    return false;
  }

  bool success = action->execute();

  if (success)
  {
    // Keep successful command for undo
    actionHistory.push_back(action);

    return true;
  }

  // Clean up unsuccessful command
  delete action;

  return false;
}

// Undo most recent successful Command
bool WorkflowEngine::undoLastAction()
{
  if (actionHistory.empty())
  {
    return false;
  }

  WorkItemAction *lastAction = actionHistory.back();

  actionHistory.pop_back();

  bool success = lastAction->undo();

  delete lastAction;

  return success;
}

// Deadline checking
void WorkflowEngine::checkDeadlines(std::time_t now)
{
  for (auto &entry : instances)
  {
    DepthFirstWorkItemIterator items(entry.second->getRoot());

    for (items.first(); !items.isDone(); items.next())
    {
      Task *task = dynamic_cast<Task *>(items.current()->undecorated());
      if (task != nullptr)
      {
        task->checkDeadline(now);
      }
    }
  }
}
