#ifndef SUBPROCESS_H
#define SUBPROCESS_H

class SubProcess : WorkItem {

public:
	vector<WorkItem*> children;

	void add(WorkItem* item);

	void remove(WorkItem* item);

	WorkItem* geChild(int index);

	int childCount();

	WorkItem* clone();

public:
	void onAvailable();

	WorkItemIterator* createIterator();
};

#endif
