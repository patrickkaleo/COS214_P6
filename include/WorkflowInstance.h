#ifndef WORKFLOWINSTANCE_H
#define WORKFLOWINSTANCE_H

#include <map>
#include <string>
#include "WorkflowDefinition.h"
#include "DependencyMediator.h"
#include "SubProcess.h"
#include "WorkflowSnapshot.h"
#include "InstanceData.h"

class ParticipantDirectory;
class WorkItemState;

class WorkflowInstance
{
private:
	std::string Id;
	const WorkflowDefinition *definition; // not owned
	DependencyMediator *mediator;					// owned
	SubProcess *rootProcess;							// owned
	InstanceData data;
	ParticipantDirectory *participants; // not owned

	WorkItem *findItemRecursive(WorkItem *current, const std::string &itemId) const;
	void captureItem(WorkItem *item,
									 std::map<std::string, std::string> &states,
									 std::map<std::string, std::string> &assignees,
									 std::map<std::string, std::string> &outcomes) const;
	void clearAssignees(WorkItem *item);
	WorkItemState *createStateFromName(const std::string &stateName) const;

public:
	WorkflowInstance(const std::string &id,
									 const WorkflowDefinition *definition,
									 const InstanceData &data);
	WorkflowInstance(const std::string &id,
									 const WorkflowDefinition *definition,
									 SubProcess *root,
									 DependencyMediator *mediator,
									 const InstanceData &data,
									 ParticipantDirectory *directory);
	~WorkflowInstance();
	WorkflowInstance(const WorkflowInstance &) = delete;
	WorkflowInstance &operator=(const WorkflowInstance &) = delete;

	bool start();
	const std::string &getId() const;
	WorkItem *findItem(const std::string &itemId) const;
	InstanceData &getData();
	std::string status() const;
	WorkflowSnapshot *createSnapshot(const std::string &label);
	void restoreSnapshot(const WorkflowSnapshot &snapshot);
};
#endif
