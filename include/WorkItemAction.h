#ifndef WORKITEMACTION_H
#define WORKITEMACTION_H

#include <string>

class WorkflowSnapshot;
class WorkflowInstance;
class WorkItem;
class Participant;

class WorkItemAction
{

protected:
	WorkflowInstance *instance;
	std::string itemId;
	Participant *actor;
	WorkflowSnapshot *before;

	virtual bool doExecute(WorkItem &item) = 0;

public:
	WorkItemAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor);
	virtual ~WorkItemAction();
	bool execute();
	bool undo();

	virtual std::string describe() const = 0;

	void description();
};

#endif
