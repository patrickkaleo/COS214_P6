#ifndef INPROGRESSSSTATE_H
#define INPROGRESSSSTATE_H

class InProgresssState : public WorkItemState
{

public:
	bool complete(WorkItem& item, string outcome);

	bool reject(WorkItem& item, string reason);

	bool cancel(WorkItem& item);

	bool escalate(WorkItem& item);

	string name();
};

#endif
