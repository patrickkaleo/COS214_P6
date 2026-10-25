#ifndef REJECTACTION_H
#define REJECTACTION_H

class RejectAction {

private:
	string reason;

protected:
	bool doeExecute(WorkItem& item);

public:
	string describe();
};

#endif
