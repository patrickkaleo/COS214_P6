#ifndef WORKITEMDECORATOR_H
#define WORKITEMDECORATOR_H

class WorkItemDecorator : WorkItem {

public:
	WorkItem* inner;

	virtual bool makeAvailable() = 0;

	bool complete(string outcome);

	bool assign(Participant* participant);

	bool start();

	bool escalate();

	bool reject(string reason);

	bool cancel();

	bool isFinished();

	string getId();

	string getName();

	string getOutcome();

	void attach(WorkflowEventListener* listener);

	void detach(WorkflowEventListener* listener);

	void notify(const WorkflowEvent& event);

	WorkItem* clone();

protected:
	void onAvailable();
};

#endif
