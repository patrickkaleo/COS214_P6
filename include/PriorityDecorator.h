#ifndef PRIORITYDECORATOR_H
#define PRIORITYDECORATOR_H


#include "WorkItemDecorator.h"
#include "WorkItem.h"

class PriorityDecorator : WorkItemDecorator {

public:
PriorityDecorator(WorkItem*, int);
	int priorityLevel;

	int getPriority();

	WorkItem* clone();

protected:
	void onAvailable();
};

#endif
