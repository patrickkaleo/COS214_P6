#ifndef DEPTHFISRTWORKITEMITERATOR_H
#define DEPTHFISRTWORKITEMITERATOR_H

class DepthFisrtWorkItemIterator : WorkItemIterator {

private:
	SubProcess* root;
	stack<WorkItem*> stack;
	WorkItem* currentItem;

public:
	void fist();

	void next();

	bool isDone();

	WorkItem* current();
};

#endif
