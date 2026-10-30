#ifndef WORKITEMITERATOR_H
#define WORKITEMITERATOR_H

class WorkItemIterator {


public:
	virtual void first() = 0;

	virtual void next() = 0;

	virtual bool isDone() = 0;

	virtual WorkItem* current() = 0;
};

#endif
