#include "ClaimAction.h"
#include "WorkItem.h"

ClaimAction::ClaimAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor)
    : WorkItemAction(instance, itemId, actor) {}

bool ClaimAction::doExecute(WorkItem &item)
{
  return item.assign(actor);
}

std::string ClaimAction::describe() const
{
  return "ClaimAction: " + itemId;
}