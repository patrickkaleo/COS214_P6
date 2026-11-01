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
	std::string operation;
	std::string keptLabel;

	virtual bool doExecute(WorkItem &item) = 0;
	virtual bool isAuthorized(const WorkItem &item) const;
	virtual std::string snapshotLabel(const WorkItem &item) const;

public:
	WorkItemAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor,
	               const std::string &operation = "");
	virtual ~WorkItemAction();
	bool execute();
	bool undo();
	/// Hands the pre-action snapshot to the caller. The action no longer deletes it.
	WorkflowSnapshot *releaseSnapshot();
	WorkflowInstance *getInstance() const { return instance; }
	std::string getSnapshotLabel() const;

	virtual std::string describe() const = 0;

	void description();
};

#endif
