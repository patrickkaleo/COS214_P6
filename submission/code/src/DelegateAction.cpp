#include "DelegateAction.h"
#include "WorkItem.h"
#include "Participant.h"
#include "WorkflowInstance.h"

DelegateAction::DelegateAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor, Participant *to)
    : WorkItemAction(instance, itemId, actor, "delegate"), to(to) {}

bool DelegateAction::doExecute(WorkItem &item)
{
  // Delegating to yourself would change nothing but still look like a handover
  if (to == nullptr || to == actor || !instance->canAssign(item, to))
  {
    return false;
  }
  return item.assign(to);
}

std::string DelegateAction::describe() const
{
  return "DelegateAction: " + itemId + ", To: " + (to ? to->getName() : "(none)");
}
