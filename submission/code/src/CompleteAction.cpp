#include "CompleteAction.h"
#include "WorkItem.h"
#include "Task.h"
#include "WorkItemDecorator.h"
#include "WorkflowInstance.h"

std::string CompleteAction::snapshotLabel(const WorkItem &item) const {
  const WorkItem *core = &item;
  while (const WorkItemDecorator *decorator = dynamic_cast<const WorkItemDecorator *>(core))
    core = decorator->getInner();
  const Task *task = dynamic_cast<const Task *>(core);
  return task && task->isCheckpoint() && !task->getServiceName().empty()
      ? "Checkpoint before " + item.getId() : describe();
}

CompleteAction::CompleteAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor, const std::string &outcome)
    : WorkItemAction(instance, itemId, actor, "complete"), outcome(outcome) {}

bool CompleteAction::doExecute(WorkItem &item)
{
  WorkItem *core = &item;
  while (WorkItemDecorator *decorator = dynamic_cast<WorkItemDecorator *>(core))
    core = decorator->getInner();
  Task *task = dynamic_cast<Task *>(core);
  if (task && !task->getServiceName().empty())
    return task->executeService(instance->getData(), outcome, &item);
  return item.complete(outcome);
}

std::string CompleteAction::describe() const
{
  return "CompleteAction: " + itemId + ", Outcome: " + outcome;
}
