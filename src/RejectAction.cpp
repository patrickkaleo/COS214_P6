#include "RejectAction.h"
#include "WorkItem.h"

RejectAction::RejectAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor, const std::string &reason)
    : WorkItemAction(instance, itemId, actor), reason(reason) {}

bool RejectAction::doExecute(WorkItem &item)
{
  // Only the assignee may reject the item
  if (item.getAssignee() != actor)
  {
    return false;
  }
  return item.reject(reason);
}

std::string RejectAction::describe() const
{
  return "RejectAction: " + itemId + ", Reason: " + reason;
}