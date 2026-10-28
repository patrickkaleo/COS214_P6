#include "DelegateAction.h"
#include "WorkItem.h"

DelegateAction::DelegateAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor, Participant *to)
    : WorkItemAction(instance, itemId, actor), to(to) {}

bool DelegateAction::doExecute(WorkItem &item)
{
  if (to == nullptr)
  {
    return false;
  }
  return item.assign(to);
}

std::string DelegateAction::describe() const
{
  return "DelegateAction: " + itemId + ", To: " + to->getName();
}