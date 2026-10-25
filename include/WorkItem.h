#ifndef WORKITEM_H
#define WORKITEM_H

class WorkItem {

protected:
	string itemID;
	string name;
private:
	RoutingStrategy* routingStrategy;
	vector<WorkflowEventListner*> listeners;
protected:
	WorkFlowMediator* mediator;
	string outcome;
	WorkItemState* state;
	SubProcess parent;

public:
	bool makeAvailable();

	bool complete(string outcome);

	bool assign(Participant* participant);

	void detach(WorkFlowEventListener* l);

	void attach(WorkflowEventListener* l);

	void notifyListeners(workFlowEvent event);

	bool start();

	bool reject(string reason);

	bool cancel();

	bool escalate();

	void setState(WoritemState* state);

	bool isFinished();

	string getId();

	string getName();

	string getOutcome();

	virtual WorkItem* clone() = 0;
	
	virtual void add(WorkItem*);
	
	virtual void setRouting(RoutingStrategy*);
	virtual void setService(string);
	virtual void setCheckointflag(bool flag);

protected:
	virtual void onAvailable() = 0;
};

#endif
