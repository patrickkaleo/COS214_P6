#include "WorkItemAction.h"
#include "WorkItem.h"
#include "WorkflowInstance.h"
#include "WorkflowSnapshot.h"

WorkItemAction::WorkItemAction(WorkflowInstance *instance, const std::string &itemId,
                             Participant *actor, const std::string &operation)
    : instance(instance), itemId(itemId), actor(actor), before(nullptr), operation(operation) {}

bool WorkItemAction::isAuthorized(const WorkItem &item) const {
  return instance && instance->canAct(item, actor, operation);
}

std::string WorkItemAction::snapshotLabel(const WorkItem &) const { return describe(); }
std::string WorkItemAction::getSnapshotLabel() const {
  return before ? before->getLabel() : describe();
}

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

  if (item == nullptr || !isAuthorized(*item))
  {
    return false;
  }

  delete before;
  before = nullptr;

  before = instance->createSnapshot(snapshotLabel(*item));

  if (before == nullptr)
  {
    return false;
  }

  bool success = false;
  try {
    success = doExecute(*item);
  } catch (...) {
    instance->restoreSnapshot(*before);
    throw;
  }

  if (!success)
  {
    instance->restoreSnapshot(*before);
    delete before;
    before = nullptr;
    return false;
  }

  instance->refreshDeadlines(std::time(nullptr));
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
