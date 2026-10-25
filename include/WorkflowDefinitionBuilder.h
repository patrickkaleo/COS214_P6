#ifndef WORKFLOWDEFINITIONBUILDER_H
#define WORKFLOWDEFINITIONBUILDER_H

#include <string>
#include <vector>
using namespace std;

#include "RoutingStrategy.h"
#include "WorkflowDefinition.h"
#include "AuditDecorator.h"
#include "AuditLogger.h"
#include "PriorityDecorator.h"
#include "SubProcess.h"
#include "Task.h"
#include "WorkItem.h"
// #include "WorkItemDepthFirstIterator.h"
#include "WorkItemIterator.h"

class WorkItem;

/**
 * @class WorkflowDefinitionBuilder
 * @brief Generic Builder for creating workflow definitions that can be applied to different scenarios and set ups
 */
class WorkflowDefinitionBuilder
{
protected:
	WorkflowDefinition *definition;

public:
	WorkflowDefinitionBuilder(string id, string name, int version);
	WorkflowDefinitionBuilder() = delete;
	virtual ~WorkflowDefinitionBuilder();
	WorkflowDefinitionBuilder(const WorkflowDefinitionBuilder &) = delete;
	/**
	 * @brief build the tree from the root
	 */
	virtual void buildStructure() = 0;

	/**
	 * @brief add extrs to the definition
	 */
	virtual void buildExtras() = 0;

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
	virtual void buildIntegrations() = 0;

	/**
	 * @brief return a workflow definition
	 * @return WorkflowDefinition*
	 */
	WorkflowDefinition *getResult();

protected:
	/**
	 * @brief add workItem to the definion, can be Subprocess of Task
	 * @param newWorkItem new WorkItem to be added
	 * @param parent parent WorkItem recieiving this new work item
	 */
	void addWorkItem(WorkItem *addition, WorkItem *parent);

	/**
	 * @brief add a dependency rule to the workflow definition
	 * @param fromId string id of where the dependency rule is from
	 * @param toId string id of the dependency rule if going to
	 * @param outcome string description of the evaluation of the dependenc yrule for a given workflow instance
	 */
	void addDependency(DependencyRule* newDepenndencyRule);

	/**
	 * @brief set routing strategy for a specific work item
	 * @param item work item for which this routing strategy applies
	 * @param strategy RoutingStrategy to be attached to the workitem
	 */
	void setRouting(WorkItem* item, RoutingStrategy *strategy);

	/**
	 * @brief add a service to a workitem
	 * @param itemID WorkItem where the service is to be added
	 * @param serviceName string name of a service to be added
	 */
	void setService(WorkItem* item, string serviceName);

	/**
	 * @brief is this work items to be checkpointed or not?
	 * @param itemId id of the item to be checkpoint
	 */
	void markCheckpoint(WorkItem* item);

	/**
	 * @brief set the priority of a work item
	 * @param itemId string id of the work item that is being given a priority
	 * @param level int level of the priority
	 */
	void addPriority(WorkItem item, int level);

	/**
	 * @brief add an audit decoration to a work item
	 * @param itemId string id of the item that is being added to the work item
	 */
	void addAudit(WorkItem* item);
};

#endif
