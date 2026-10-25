#ifndef DELEGATEACTION_H
#define DELEGATEACTION_H

class DelegateAction {

private:
	Participant* to;

public:
	bool doExecute(WorkItem& item);

	string describe();
};

#endif
