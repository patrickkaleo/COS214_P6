#include "CancelAction.h"
#include "WorkItem.h"

CancelAction::CancelAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor)
    : WorkItemAction(instance, itemId, actor, "cancel") {}

bool CancelAction::doExecute(WorkItem &item)
{
  return item.cancel();
}

std::string CancelAction::describe() const
{
  return "CancelAction: " + itemId;
}
