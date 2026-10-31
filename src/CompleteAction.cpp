#include "CompleteAction.h"
#include "WorkItem.h"

CompleteAction::CompleteAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor, const std::string &outcome)
    : WorkItemAction(instance, itemId, actor), outcome(outcome) {}

bool CompleteAction::doExecute(WorkItem &item)
{
  // Only the assignee may complete the item
  if (item.getAssignee() != actor)
  {
    return false;
  }
  return item.complete(outcome);
}

std::string CompleteAction::describe() const
{
  return "CompleteAction: " + itemId + ", Outcome: " + outcome;
}