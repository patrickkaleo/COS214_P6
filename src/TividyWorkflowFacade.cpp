
#include "TividyWorkflowFacade.h"

#include "WorkflowDefinitionBuilder.h"
#include "WorkflowDefinition.h"
#include "WorkflowInstance.h"

#include "Participant.h"
#include "ExternalService.h"
#include "WorkflowEventListener.h"
#include "EventDispatcher.h"
#include "FileWorkflowDefinitionBuilder.h"

#include <fstream>
#include <exception>

#include "ClaimAction.h"
#include "StartAction.h"
#include "CompleteAction.h"
#include "RejectAction.h"
#include "DelegateAction.h"
#include "CancelAction.h"

bool TividyWorkflowFacade::runAction(WorkItemAction *action) {
  const bool success = engine.executeAction(action);
  lastError = engine.getLastError();
  return success;
}

WorkflowInstance *TividyWorkflowFacade::getInstance(const std::string &id) const {
  return engine.getInstance(id);
}
std::vector<std::string> TividyWorkflowFacade::definitionIds() const { return engine.definitionIds(); }
std::vector<std::string> TividyWorkflowFacade::instanceIds() const { return engine.instanceIds(); }
std::vector<std::string> TividyWorkflowFacade::actionHistory() const { return engine.historyDescriptions(); }
Participant *TividyWorkflowFacade::findParticipant(const std::string &id) const {
  return engine.getDirectory().findById(id).get();
}
std::vector<Participant *> TividyWorkflowFacade::participants() const {
  std::vector<Participant *> result;
  for (const auto &participant : engine.getDirectory().getAll()) result.push_back(participant.get());
  return result;
}
std::vector<std::string> TividyWorkflowFacade::availableActions(const std::string &instanceId,
                                                              const std::string &itemId,
                                                              Participant *actor) const {
  WorkflowInstance *instance = engine.getInstance(instanceId);
  return instance ? instance->availableActions(itemId, actor) : std::vector<std::string>();
}

// Helper: construct definition using Builder pattern.
std::string TividyWorkflowFacade::registerDefinition(
    WorkflowDefinitionBuilder &builder)
{
  lastError.clear();
  try {
    WorkflowDefinition *definition = director.construct(&builder);
    const std::string id = engine.registerDefinition(definition);
    if (id.empty()) lastError = engine.getLastError();
    return id;
  } catch (const std::exception &error) {
    lastError = error.what();
    return "";
  }
}

std::string TividyWorkflowFacade::registerDefinitionFromFile(const std::string &filePath)
{
  lastError.clear();
  std::ifstream input(filePath.c_str());

  if (!input.is_open())
  {
    lastError = "Cannot open workflow definition: " + filePath;
    return "";
  }

  input.close();

  try {
    FileWorkflowDefinitionBuilder builder(filePath);
    return registerDefinition(builder);
  } catch (const std::exception &error) {
    lastError = error.what();
    return "";
  }
}

// Add a participant to the directory.

void TividyWorkflowFacade::addParticipant(
    const std::string &id,
    const std::string &name,
    const std::vector<std::string> &roles)
{
  lastError.clear();
  if (id.empty() || name.empty() || engine.getDirectory().findById(id)) {
    lastError = "Participant ID and name are required, and the ID must be unique.";
    return;
  }
  Participant *participant = new Participant(
      id,
      name,
      roles.empty() ? "" : roles[0]);

  for (size_t i = 1; i < roles.size(); ++i)
  {
    participant->addRole(roles[i]);
  }

  engine.getDirectory().addParticipant(participant);
}

// Register an external system adapter.
void TividyWorkflowFacade::addService(
    ExternalService *service)
{

  if (service != nullptr)
  {
    engine.getServiceRegistry().add(service);
  }
}

// Register an event listener.
void TividyWorkflowFacade::addListener(
    WorkflowEventListener *listener)
{

  if (listener != nullptr)
  {
    // Engine must retain ownership of listener.
    engine.addListener(listener);
  }
}

// Create and start a workflow instance.
std::string TividyWorkflowFacade::startInstance(
    const std::string &defId,
    const InstanceData &data)
{
  lastError.clear();
  WorkflowInstance *instance =
      engine.createInstance(defId, data);

  if (instance == nullptr)
  {
    lastError = engine.getLastError();
    return "";
  }

  if (!instance->start()) {
    lastError = "The workflow could not start: its root is invalid or already started.";
    return "";
  }

  return instance->getId();
}

// Claim a work item.
bool TividyWorkflowFacade::claim(
    const std::string &instId,
    const std::string &itemId,
    Participant *actor)
{

  WorkflowInstance *instance =
      engine.getInstance(instId);

  if (instance == nullptr || actor == nullptr)
  {
    lastError = "Unknown instance or missing actor.";
    return false;
  }

  return runAction(
      new ClaimAction(instance, itemId, actor));
}

// Start work on an item.
bool TividyWorkflowFacade::start(
    const std::string &instId,
    const std::string &itemId,
    Participant *actor)
{

  WorkflowInstance *instance =
      engine.getInstance(instId);

  if (instance == nullptr || actor == nullptr)
  {
    lastError = "Unknown instance or missing actor.";
    return false;
  }

  return runAction(
      new StartAction(instance, itemId, actor));
}

// Complete a work item.
bool TividyWorkflowFacade::complete(
    const std::string &instId,
    const std::string &itemId,
    Participant *actor,
    const std::string &outcome)
{

  WorkflowInstance *instance =
      engine.getInstance(instId);

  if (instance == nullptr || actor == nullptr)
  {
    lastError = "Unknown instance or missing actor.";
    return false;
  }

  return runAction(
      new CompleteAction(
          instance, itemId, actor, outcome));
}

// Reject a work item.
bool TividyWorkflowFacade::reject(
    const std::string &instId,
    const std::string &itemId,
    Participant *actor,
    const std::string &reason)
{

  WorkflowInstance *instance =
      engine.getInstance(instId);

  if (instance == nullptr || actor == nullptr)
  {
    lastError = "Unknown instance or missing actor.";
    return false;
  }

  return runAction(
      new RejectAction(
          instance, itemId, actor, reason));
}

// Delegate work to someone else.
bool TividyWorkflowFacade::delegate(
    const std::string &instId,
    const std::string &itemId,
    Participant *actor,
    Participant *to)
{

  WorkflowInstance *instance =
      engine.getInstance(instId);

  if (instance == nullptr ||
      actor == nullptr || to == nullptr)
  {
    lastError = "Unknown instance or missing actor.";
    return false;
  }

  return runAction(
      new DelegateAction(instance, itemId, actor, to));
}

// Cancel a work item.
bool TividyWorkflowFacade::cancel(
    const std::string &instId,
    const std::string &itemId,
    Participant *actor)
{

  WorkflowInstance *instance =
      engine.getInstance(instId);

  if (instance == nullptr || actor == nullptr)
  {
    lastError = "Unknown instance or missing actor.";
    return false;
  }

  return runAction(
      new CancelAction(instance, itemId, actor));
}

// Undo most recent successful action.
bool TividyWorkflowFacade::undoLast()
{
  const bool success = engine.undoLastAction();
  lastError = engine.getLastError();
  return success;
}

// Delegate deadline handling to the engine.
void TividyWorkflowFacade::checkDeadlines(std::time_t now)
{
  engine.checkDeadlines(now);
}

// Retrieve current instance status.
std::string TividyWorkflowFacade::getStatus(
    const std::string &instId)
{

  WorkflowInstance *instance =
      engine.getInstance(instId);

  if (instance == nullptr)
  {
    return "not found";
  }

  return instance->status();
}
