#include "DelegateAction.h"
#include "WorkItem.h"
#include "Participant.h"

DelegateAction::DelegateAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor, Participant *to)
    : WorkItemAction(instance, itemId, actor), to(to) {}

bool DelegateAction::doExecute(WorkItem &item)
{
  // Only the assignee may hand the item to someone else
  if (to == nullptr || item.getAssignee() != actor)
  {
    return false;
  }
  return item.assign(to);
}

std::string DelegateAction::describe() const
{
  return "DelegateAction: " + itemId + ", To: " + to->getName();
}