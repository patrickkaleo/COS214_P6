#ifndef PRIORITYDECORATOR_H
#define PRIORITYDECORATOR_H

class PriorityDecorator : WorkItemDecorator {

public:
	int priorityLevel;

	int getPriority();

	WorkItem* clone();

protected:
	void onAvailable();
};

#endif
