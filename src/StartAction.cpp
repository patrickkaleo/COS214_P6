#include "StartAction.h"
#include "WorkItem.h"

StartAction::StartAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor)
    : WorkItemAction(instance, itemId, actor) {}

bool StartAction::doExecute(WorkItem &item)
{
  return item.start();
}

std::string StartAction::describe() const
{
  return "StartAction: " + itemId;
}