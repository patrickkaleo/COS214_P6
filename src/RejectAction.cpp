#include "RejectAction.h"
#include "WorkItem.h"

RejectAction::RejectAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor, const std::string &reason)
    : WorkItemAction(instance, itemId, actor), reason(reason) {}

bool RejectAction::doExecute(WorkItem &item)
{
  return item.reject(reason);
}

std::string RejectAction::describe() const
{
  return "RejectAction: " + itemId + ", Reason: " + reason;
}