#ifndef WORKFLOWSNAPSHOT_H
#define WORKFLOWSNAPSHOT_H

class WorkflowSnapshot {

private:
	string label;
	vector<string> savedPayload;
	InstanceData data;
	map<string, string> outcome;
	map<string, string> assignees;
	map<string, string> itemStates;

public:
	string getLabel();
};

#endif
