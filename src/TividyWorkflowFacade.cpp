
#include "TividyWorkflowFacade.h"

#include "WorkflowDefinitionBuilder.h"
#include "WorkflowDefinition.h"
#include "WorkflowInstance.h"

#include "Participant.h"
#include "ExternalService.h"
#include "WorkflowEventListener.h"
#include "EventDispatcher.h"
#include "FileWorkflowDefinition"

#include <fstream>

#include "ClaimAction.h"
#include "StartAction.h"
#include "CompleteAction.h"
#include "RejectAction.h"
#include "DelegateAction.h"
#include "CancelAction.h"

// Helper: construct definition using Builder pattern.
std::string TividyWorkflowFacade::registerDefinition(
    WorkflowDefinitionBuilder &builder)
{

  WorkflowDefinition *definition =
      director.construct(&builder);

  if (definition == nullptr)
  {
    return "";
  }

  // Engine takes ownership of the definition.
  return engine.registerDefinition(definition);
}

std::string TividyWorkflowFacade::registerDefinitionFromFile(const std::string &filePath){
    std::ifstream input(filePath.c_str());

    if(!input.is.open()){
      return "unable to open file";
    }

    input.close();

    FileWorkflowDefinitionBuilder builder(filePath);

    return registerDefinition(builder);
}

// Add a participant to the directory.
void TividyWorkflowFacade::addParticipant(
    const std::string &id,
    const std::string &name,
    const std::vector<std::string> &roles)
{

  Participant *participant =
      new Participant(id, name, roles);

  engine.getDirectory().add(participant);
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

  WorkflowInstance *instance =
      engine.createInstance(defId, data);

  if (instance == nullptr)
  {
    return "";
  }

  instance->start();

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
    return false;
  }

  return engine.executeAction(
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
    return false;
  }

  return engine.executeAction(
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
    return false;
  }

  return engine.executeAction(
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
    return false;
  }

  return engine.executeAction(
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
    return false;
  }

  return engine.executeAction(
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
    return false;
  }

  return engine.executeAction(
      new CancelAction(instance, itemId, actor));
}

// Undo most recent successful action.
bool TividyWorkflowFacade::undoLast()
{
  return engine.undoLast();
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
