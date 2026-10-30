
#include "WorkflowEngine.h"
#include "WorkflowInstance.h"
#include "WorkflowDefinition.h"
// #include "f.h"
#include "InstanceData.h"
#include "WorkItemAction.h"
#include "WorkflowEventListener.h"
#include "EventDispatcher.h"

#include <string>

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

  // Generate unique instance ID
  std::string id =
      "instance-" + std::to_string(nextInstanceNumber++);

  // Create instance from registered definition
  WorkflowInstance *instance =
      new WorkflowInstance(id, found->second, data);

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
  // Deadline handling must be connected to the
  // WorkflowInstance deadline implementation.

  (void)now;
}
