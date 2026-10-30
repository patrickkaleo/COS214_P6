#ifndef WORKFLOWMEDIATOR_H
#define WORKFLOWMEDIATOR_H

#include "WorkItem.h"
#include "SubProcess.h"
class WorkflowMediator
{

public:
	// void itemChanged(WorkItem &item);

	virtual void itemChanged(WorkItem &i) = 0;

	virtual void openChildren(SubProcess &s) = 0;
};

#endif
