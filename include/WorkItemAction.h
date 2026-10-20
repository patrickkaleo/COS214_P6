#ifndef WORKITEMACTION_H
#define WORKITEMACTION_H

class WorkItemAction {

private:
	WorkflowInstance* Instance;
	string ItemId;
	Participant* actor;
	WorkFlowSnapshot* before;

public:
	bool doExecute(WorkItem& item, WorkItem& item);

	virtual string describe() = 0;

	void description();
};

#endif
