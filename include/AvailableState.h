#ifndef AVAILABLESTATE_H
#define AVAILABLESTATE_H

class AvailableState {


public:
	bool assign(WorkItem& item, Participant* participant);

	bool cancel(WorkItem& Item);

	bool escalate(WorkItem& item);

	string name();
};

#endif
