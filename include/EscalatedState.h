#ifndef ESCALATEDSTATE_H
#define ESCALATEDSTATE_H

class EscalatedState {


public:
	bool assign(WorkItem& item, Participant* participant);

	bool cancel(WorkItem& item);

	string name();
};

#endif
