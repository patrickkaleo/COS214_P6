#ifndef WORKFLOWENGINE_H
#define WORKFLOWENGINE_H

class WorkflowEngine {

private:
	vector<WorkItemAction*> actionHistory;
	map<string, WorkflowDefinition*> definition;
	map<string, workflowInstance*> instances;
	ServiceRegistry services;
	HistoryLog history;
	vector<WorkFlowEventListener*> listeners;

public:
	string registerDefinition(WorkflowDefinition* definition);

	WorkflowInstance* createInstance(string definitionId, InstanceData data);

	bool executeAction(WorkItemAction* action);

	bool undoLast();

	void checkDeadlines(time_t now);

	WorkFlowInstance* getInstance(string id);
};

#endif
