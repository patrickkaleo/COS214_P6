#ifndef WORKFLOWINSTANCE_H
#define WORKFLOWINSTANCE_H

#include <string>
#include "WorkflowDefinition.h"
#include "DependencyMediator.h"
#include "SubProcess.h"
#include "WorkflowSnapshot.h"

class WorkflowInstance
{

private:
	std::string Id;
	const WorkflowDefinition *definition;
	DependencyMediator mediator;
	SubProcess *rootProcess;
	InstanceData data;

public:
	WorkflowInstance(
			const std::string &id,
			const WorkflowDefinition *definition,
			const InstanceData &data);
	bool start();

	std::string getId();
	WorkItem *findItem(std::string id);

	InstanceData getData();

	std::string status();

	WorkflowSnapshot *createSnapshot(std::string label);

	void restoreSnapshot(const WorkflowSnapshot &snapshot);
};

#endif
