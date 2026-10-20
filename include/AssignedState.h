#ifndef ASSIGNEDSTATE_H
#define ASSIGNEDSTATE_H

class AssignedState {


public:
	bool start(WorkItem& item);

	bool assign(WorkItem& item, Participant* participant);

	bool cancel(WorkItem& item);

	bool escalate(WorkItem& item);

	string name();
};

#endif
