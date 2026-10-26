#ifndef WORKFLOWENGINE_H
#define WORKFLOWENGINE_H

#include <vector>

class WorkItemAction;

class WorkflowEngine
{
private:
  std::vector<WorkItemAction *> actionHistory;

public:
  bool executeAction(WorkItemAction *action);
  bool undoLastAction();
};

#endif