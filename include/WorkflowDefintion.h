#ifndef WORKFLOWDEFINTION_H
#define WORKFLOWDEFINTION_H

class WorkflowDefinition {

private:
	string definitonId;
	string name;
	int version;
	subProcess* rootProcess;

public:
	const subProcess* getRoot();

	const vector<DependencyRule> getRules();

	int getVersion();

	subProcess* instantiateTree();
};

#endif
