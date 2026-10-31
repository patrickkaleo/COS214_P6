#include "StartAction.h"
#include "WorkItem.h"
#include "Task.h"
#include "WorkflowInstance.h"

StartAction::StartAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor)
    : WorkItemAction(instance, itemId, actor) {}

bool StartAction::doExecute(WorkItem &item)
{
  // Only the assignee may start the item
  if (item.getAssignee() != actor || !item.start())
  {
    return false;
  }

  Task *task = dynamic_cast<Task *>(item.undecorated());
  if (task != nullptr)
  {
    task->performWork(instance->getData());
  }
  return true;
}

std::string StartAction::describe() const
{
  return "StartAction: " + itemId;
}