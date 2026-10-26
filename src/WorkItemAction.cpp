#include "WorkItemAction.h"
#include "WorkItem.h"
#include "WorkflowInstance.h"
#include "WorkFlowSnapshot.h"

WorkItemAction::WorkItemAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor) : instance(instance), itemId(itemId), actor(actor), before(nullptr) {}

WorkItemAction::~WorkItemAction()
{
  delete before;
}

bool WorkItemAction::execute()
{
  if (instance == nullptr)
  {
    return false;
  }

  WorkItem *item = instance->findItem(itemId);

  if (item == nullptr)
  {
    return false;
  }

  delete before;
  before = nullptr;

  before = instance->createSnapshot(describe());

  if (before == nullptr)
  {
    return false;
  }

  bool success = doExecute(*item);

  if (!success)
  {
    delete before;
    before = nullptr;
    return false;
  }

  return true;
}

bool WorkItemAction::undo()
{
  if (instance == nullptr || before == nullptr)
  {
    return false;
  }

  instance->restoreSnapshot(*before);

  return true;
}