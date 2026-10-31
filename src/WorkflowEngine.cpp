
#include "WorkflowEngine.h"
#include "WorkflowInstance.h"
#include "WorkflowDefinition.h"
// #include "f.h"
#include "InstanceData.h"
#include "WorkItemAction.h"
#include "WorkflowEventListener.h"
#include "EventDispatcher.h"
#include "WorkflowDefinitionBuilder.h"
#include "DepthFirstWorkItemIterator.h"
#include <exception>
#include <memory>

#include <string>

namespace {
// Deadline processing is a system command, so its complete local state can be
// undone just like a participant command. It is not exposed as a user action.
class DeadlineAction : public WorkItemAction {
  std::time_t now;
protected:
  bool isAuthorized(const WorkItem &) const override { return true; }
  bool doExecute(WorkItem &) override {
    instance->checkDeadlines(now);
    return true;
  }
public:
  DeadlineAction(WorkflowInstance *instance, std::time_t now)
      : WorkItemAction(instance, instance->getRoot()->getId(), nullptr), now(now) {}
  std::string describe() const override { return "Check deadlines: " + instance->getId(); }
};
}

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
  lastError.clear();
  if (definition == nullptr)
  {
    lastError = "No workflow definition was produced.";
    return "";
  }

  std::string id = definition->getId();

  const auto existing = definitions.find(id);
  if (existing != definitions.end() && existing->second == definition) {
    lastError = "This workflow definition is already registered.";
    return "";
  }

  // Prevent empty or duplicate IDs
  if (id.empty() ||
      definitions.find(id) != definitions.end())
  {
    delete definition;
    lastError = "The workflow definition ID is empty or already registered.";
    return "";
  }

  try {
    WorkflowDefinitionBuilder::validateDefinition(definition);
  } catch (const std::exception &error) {
    lastError = error.what();
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
  lastError.clear();
  auto found = definitions.find(definitionId);

  if (found == definitions.end())
  {
    lastError = "Unknown workflow definition: " + definitionId;
    return nullptr;
  }

  // Generate unique instance ID
  std::string id =
      "instance-" + std::to_string(nextInstanceNumber++);

  // Create instance from registered definition
  WorkflowInstance *instance =
      new WorkflowInstance(id, found->second, data, &directory, &services);

  // Engine takes ownership
  instances[id] = instance;

  return instance;
}

// Retrieve a workflow instance
WorkflowInstance *WorkflowEngine::getInstance(
    const std::string &instanceId) const
{
  auto found = instances.find(instanceId);

  if (found == instances.end())
  {
    return nullptr;
  }

  return found->second;
}

std::vector<std::string> WorkflowEngine::definitionIds() const {
  std::vector<std::string> result;
  for (const auto &entry : definitions) result.push_back(entry.first);
  return result;
}

std::vector<std::string> WorkflowEngine::instanceIds() const {
  std::vector<std::string> result;
  for (const auto &entry : instances) result.push_back(entry.first);
  return result;
}

std::vector<std::string> WorkflowEngine::historyDescriptions() const {
  std::vector<std::string> result;
  for (WorkItemAction *action : actionHistory) {
    result.push_back(action->getInstance()->getId() + ": " + action->getSnapshotLabel());
  }
  return result;
}

// Get participant directory
ParticipantDirectory &WorkflowEngine::getDirectory()
{
  return directory;
}

const ParticipantDirectory &WorkflowEngine::getDirectory() const { return directory; }

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
  lastError.clear();
  if (action == nullptr)
  {
    return false;
  }

  WorkflowInstance *owner = action->getInstance();
  if (!owner || getInstance(owner->getId()) != owner) {
    lastError = "The command does not belong to this engine.";
    delete action;
    return false;
  }
  bool success = false;
  try {
    success = action->execute();
  } catch (const std::exception &error) {
    lastError = error.what();
  }

  if (success)
  {
    // Keep successful command for undo
    actionHistory.push_back(action);

    return true;
  }

  // Clean up unsuccessful command
  delete action;

  if (lastError.empty()) lastError = "The actor is not authorized or the action is invalid in the current state.";

  return false;
}

// Undo most recent successful Command
bool WorkflowEngine::undoLastAction()
{
  lastError.clear();
  if (actionHistory.empty())
  {
    lastError = "There is no successful action to undo.";
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
  for (const auto &entry : instances) {
    WorkflowInstance *instance = entry.second;
    instance->refreshDeadlines(now);
    bool expired = false;
    DepthFirstWorkItemIterator it(instance->getRoot());
    for (it.first(); !it.isDone(); it.next()) {
      const std::time_t deadline = instance->getDeadline(it.current()->getId());
      if (deadline != 0 && deadline <= now) { expired = true; break; }
    }
    if (expired) executeAction(new DeadlineAction(instance, now));
  }
}
