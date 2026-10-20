#ifndef COMPLETEACTION_H
#define COMPLETEACTION_H

class CompleteAction : WorkItemAction {

public:
	string outcome;

	bool doeExecute(WorkItem& Item);

	string describe();
};

#endif
