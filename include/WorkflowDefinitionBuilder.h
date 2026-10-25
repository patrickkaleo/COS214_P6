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
	 * @brief build the tree structure with work items that a work flow instance will go through
	 * @brief the work tree is from the composite pattern desing on the WorkItem
	 */
	virtual void buildDefinition() = 0;

	WorkflowDefinition *getResult();

protected:
	/**
	 * @brief add workItem to the definion, can be Subprocess of Task
	 * @param newWorkItem
	 * @param parent
	 */
	void addWorkItem(WorkItem *newWorkItem, WorkItem *parent);

	/**
	 * @brief add a dependency rule to the workflow definition
	 * @param newDependencyrule
	 */
	void addDependency(DependencyRule *newDepenndencyRule);

	/**
	 * @brief set routing strategy for a specific work item
	 * @param item
	 * @param strategy
	 * */
	void setRouting(WorkItem *item, RoutingStrategy *strategy);

	/**
	 * @brief add a service to a workitem
	 * @param item WorkItem where the service is to be added
	 * @param serviceName string name of a service to be added
	 */
	void setService(WorkItem *item, string serviceName);

	/**
	 * @brief is this work items to be checkpointed or not?
	 * @param item Item whose flag for checkpointing is being raised
	 */
	void markCheckpoint(WorkItem *item);

	/**
	 * @brief set the priority of a work item
	 * @param item work item that is being given a priority
	 * @param level int level of the priority
	 */
	void addPriority(WorkItem* item, int level);

	/**
	 * @brief add an audit decoration to a work item
	 * @param item item that is being added to the work item
	 */
	void addAudit(WorkItem *item);
};

#endif
