#ifndef WORKFLOWMEDIATOR_H
#define WORKFLOWMEDIATOR_H

class WorkflowMediator {


public:
	void itemChanged(WorkItem& item);

	virtual void itemChanged(Workitem& i) = 0;

	virtual void openChildren(Subprocess& s) = 0;
};

#endif
