#ifndef STARTACTION_H
#define STARTACTION_H

class StartAction {


public:
	bool doExecute(WorkItem& item);

	string describe();
};

#endif
