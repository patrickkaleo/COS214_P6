#ifndef WORKFLOWINSTANCE_H
#define WORKFLOWINSTANCE_H

class WorkFlowInstance {

private:
	string Id;
	const WorkFlowDefinition* definition;
	DependencyMediator mediator;
	SubProcess* rootProcess;
	InstanceData data;

public:
	bool start();

	WorkItem* findItem(string id);

	InstanceData getDatat();

	string status();

	WorkflowSnapshot* createSnapshot(string label);

	void restoreSnapshot(constWorkflowSnapshot& snapshot);
};

#endif
