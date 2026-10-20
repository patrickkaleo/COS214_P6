#ifndef CREATESTATE_H
#define CREATESTATE_H

class createState {


public:
	bool makeAvailable(WorkItem& item);

	bool cancel(WorkItem& item);

	string name();
};

#endif
