#ifndef WORKFLOWINSTANCE_H
#define WORKFLOWINSTANCE_H
#include "WorkflowDefintion.h"
#include "SubProcess.h"
#include "DependencyMediator.h"
#include "InstanceData.h"
#include "WorkflowSnapshot.h"
#include "WorkItem.h"
#include "Task.h"
#include "ParticipantDirectory.h"
#include<map>
#include<vector>

using namespace std;
class WorkFlowInstance {

private:
	string Id;
	const WorkflowDefinition* definition;
	DependencyMediator* mediator;
	SubProcess* rootProcess;
	InstanceData data;
	ParticipantDirectory* particpants;
public:
	WorkFlowInstance(const string & id,  const WorkflowDefinition* definition, SubProcess* root, DependencyMediator* mediator,const InstanceData& data, ParticipantDirectory* directory);
	
	~WorkFlowInstance();

	bool start();

	WorkItem* findItem( const string& id)const;

	const string& getId() const;

	InstanceData& getData();

	string status() const;

	WorkflowSnapshot* createSnapshot(const string& label);
	
	WorkItem* findItemRecursive(WorkItem* current,const std::string& itemId) const;

	void captureItem(WorkItem* item,map<string, string>& states,map<string, string>& assignees, map<string, string>& outcomes)const;

	void restoreSnapshot(const WorkflowSnapshot& snapshot);
	
	void clearAssignees(WorkItem* item);

	WorkItemState* createStateFromName(const string& stateName) const;
};

#endif
