#ifndef WORKITEMSTATE_H
#define WORKITEMSTATE_H

class WorkItemState {


public:
	bool makeAvailable(WorkItem& item);

	bool assign(WorkItem& item, Participant* participant);

	bool start(WorkItem& item);

	bool complete(WorkItem& item, string outcome);

	bool reject(WorkItem& item, string reason);

	bool cancel(WorkItem& item);

	bool escalate(WorkItem& item);

	virtual string name() = 0;
};

#endif
