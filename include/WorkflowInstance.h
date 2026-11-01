#ifndef WORKFLOWINSTANCE_H
#define WORKFLOWINSTANCE_H

#include <map>
#include <string>
#include <ctime>
#include <vector>
#include "WorkflowDefinition.h"
#include "DependencyMediator.h"
#include "SubProcess.h"
#include "WorkflowSnapshot.h"
#include "InstanceData.h"

class ParticipantDirectory;
class WorkItemState;
class ServiceRegistry;
class Participant;

class WorkflowInstance
{
private:
	std::string Id;
	const WorkflowDefinition *definition; // not owned
	DependencyMediator *mediator;					// owned
	WorkItem *rootProcess; // owned, including any decorator around the root
	InstanceData data;
	ParticipantDirectory *participants; // not owned
	std::map<std::string, std::time_t> deadlines;
	std::map<std::string, std::string> deadlineOwners;
	void wireRuntime(ServiceRegistry *services);

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
	WorkflowInstance(const std::string &id, const WorkflowDefinition *definition,
	                 const InstanceData &data, ParticipantDirectory *directory,
	                 ServiceRegistry *services);
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
	const InstanceData &getData() const;
	WorkItem *getRoot() const;
	std::string getDefinitionId() const;
	bool isRegisteredParticipant(const Participant *actor) const;
	bool canAssign(const WorkItem &item, const Participant *actor) const;
	bool canAct(const WorkItem &item, const Participant *actor,
	            const std::string &action) const;
	std::vector<std::string> availableActions(const std::string &itemId,
	                                          const Participant *actor) const;
	void refreshDeadlines(std::time_t now);
	void checkDeadlines(std::time_t now);
	std::time_t getDeadline(const std::string &itemId) const;
	std::string status() const;
	WorkflowSnapshot *createSnapshot(const std::string &label);
	void restoreSnapshot(const WorkflowSnapshot &snapshot);
};
#endif
