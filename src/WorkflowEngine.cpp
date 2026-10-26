#include "WorkflowEngine.h"
#include "WorkItemAction.h"

bool WorkflowEngine::executeAction(WorkItemAction *action)
{
  if (action == nullptr)
  {
    return false;
  }

  bool success = action->execute();

  if (success)
  {
    actionHistory.push_back(action);
    return true;
  }

  delete action;
  return false;
}

bool WorkflowEngine::undoLastAction()
{
  if (actionHistory.empty())
  {
    return false;
  }

  WorkItemAction *lastAction = actionHistory.back();

  actionHistory.pop_back();

  bool success = lastAction->undo();

  delete lastAction;

  return success;
}