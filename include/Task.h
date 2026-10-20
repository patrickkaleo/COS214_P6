#ifndef TASK_H
#define TASK_H

class Task : WorkItem {

private:
	Participant* assignee;
	bool checkpoint;
	int escalationLevel;
	string serviceName;
	RoutingStrategy* routing;

public:
	Participant* getAssignee();

	void performWork(InstanceContext& context);

	WorkItem* clone();

protected:
	void onAvailable();
};

#endif
