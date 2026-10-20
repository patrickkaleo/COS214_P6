#ifndef CANCELACTION_H
#define CANCELACTION_H

class CancelAction : WorkItemAction {


public:
	bool doExecute(WorkItem& Item);

	string describe();
};

#endif
