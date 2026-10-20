#ifndef WORKFLOWDEFINITIONBUILDER_H
#define WORKFLOWDEFINITIONBUILDER_H

#include <string>
using namespace std;

#include "WorkflowDefintion.h"
#include "RoutingStrategy.h"
#include "WorkItem.h"

/**
 * @class WorkflowDefinitionBuilder
 * @brief Generic Builder for creating workflow definitions that can be applied to different scenarios and set ups
 */
class WorkflowDefinitionBuilder
{

protected:
	WorkflowDefinition *definition;

public:
	/**
	 * @brief reset name and version of workflow definition
	 * @param name new name
	 * @param version new version
	 */
	void reset(string name, int version);

	/**
	 * @brief
	 */
	virtual void buildstructure() = 0;

	/**
	 * @brief build the relevant dependency rules for the work definition
	 */
	virtual void buildDependencies() = 0;

	/**
	 * @brief build the routing strategy for the workflow definition
	 * @brief Examples of routing: RoleBasedRouting, DataRuleRouting, ApprovalRouting
	 */
	virtual void buildRouting() = 0;

	/**
	 * @brief build integrations for the workflow definitions
	 */
	virtual void buildIntergrations() = 0;

	/**
	 * @brief return a workflow definition
	 * @return WorkflowDefinition*
	 */
	WorkflowDefinition *getResult();

protected:
	/**
	 * @brief add sub process to work definition
	 * @param id subprocess id
	 * @param name subprocess name
	 * @param parentId of subprocess that is parent to the new subprocess to be added
	 */
	void addSubProcess(string id, string name, string parentId);

	/**
	 * @brief add task item to the workflow definition @see WorkItem
	 * @param id string id of the work item  
	 * @param name string name of the work item 
	 * @param parentid string id of the parent containing this work item
	 */
	void addTask(string id, string name, string parentId);

	/**
	 * @brief add a dependency rule to the workflow definition
	 * @param fromId string id of where the dependency rule is from 
	 * @param toId string id of the dependency rule if going to
	 * @param outcome string description of the evaluation of the dependenc yrule for a given workflow instance 
	 */
	void addDependency(string fromId, string toId, string outcome);

	/**
	 * @brief Build the sequence for execution of the workflow 
	 * @param ids vector list of ids that have to be executed in the given order 
	 */
	void addSequence(vector<string> ids);

	/**
	 * @brief set routing strategy for a specific work item 
	 * @param itemId string id of the work item for which this routing strategy applies 
	 * @param strategy RoutingStrategy to be attached to the workitem
	 */
	void setRouting(string itemId, RoutingStrategy *strategy);

	/**
	 * @brief add a service to a workitem 
	 * @param itemID WorkItem where the service is to be added
	 * @param serviceName string name of a service to be added  
	 */
	void setService(string itemId, string serviceName);

	/** 
	 * @brief is this work items to be checkpointed or not?
	 * @param itemId id of the item to be checkpoint
	 */
	void markCheckpoint(string itemId);

	/**
	 * @brief set the priority of a work item
	 * @param itemId string id of the work item that is being given a priority 
	 * @param level int level of the priority 
	 */
	void addPriority(string itemId, int level);

	/**
	 * @brief add an audit decoration to a work item
	 * @param itemId string id of the item that is being added to the work item
	 */
	void addAudit(string itemId);
};

#endif
