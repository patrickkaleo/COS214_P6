#ifndef WORKFLOWDEFINITIONBUILDER_H
#define WORKFLOWDEFINITIONBUILDER_H

#include <map>
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

/**
 * @class WorkflowDefinitionBuilder
 * @brief Generic Builder for creating workflow definitions that can be applied to different scenarios and set ups.
 * The director calls the four section steps in order. Helpers refer to work items by id.
 */
class WorkflowDefinitionBuilder
{
protected:
	/**
	 * @brief Definition being assembled. Null after getResult() hands it to the caller.
	 */
	WorkflowDefinition *definition;
	map<string, WorkItem*> items; // for finding items in o(logn)

public:
	/**
	 * @brief A builder must be given an id, or it must read one from a file.
	 */
	WorkflowDefinitionBuilder();

	/**
	 * @brief Delete a definition that was never handed over by getResult().
	 */
	virtual ~WorkflowDefinitionBuilder();

	WorkflowDefinitionBuilder(const WorkflowDefinitionBuilder &) = delete;
	WorkflowDefinitionBuilder &operator=(const WorkflowDefinitionBuilder &) = delete;

	/**
	 * @brief Build the composite work-item tree, starting at the root.
	 */
	virtual void buildStructure() = 0;

	/**
	 * @brief Add dependency rules. Each rule names a from id, a to id and an outcome.
	 */
	virtual void buildDependencies() = 0;

	/**
	 * @brief Attach services and routing strategies to work items by id.
	 */
	virtual void buildServices() = 0;

	/**
	 * @brief Mark checkpoints and wrap items with priority and audit decorators.
	 */
	virtual void buildExtras() = 0;

	/**
	 * @brief Hand the finished definition to the caller. The builder no longer owns it.
	 * @return WorkflowDefinition* the product, or nullptr when nothing was built
	 */
	WorkflowDefinition *getResult();

protected:
	/**
	 * @brief Create the product after the id, name and version are known.
	 * @param id definition id
	 * @param name human-readable name
	 * @param version version number
	 */
	void startDefinition(string id, string name, int version);

	/**
	 * @brief Add a subprocess. Parent id "-" makes this process the root.
	 * @param id subprocess id
	 * @param name subprocess name
	 * @param parentId id of the parent process, or "-" for the root
	 */
	void addProcess(string id, string name, string parentId);

	/**
	 * @brief Add a task under the subprocess with the given id.
	 * @param id task id
	 * @param name task name
	 * @param parentId id of the parent subprocess
	 */
	void addTask(string id, string name, string parentId);

	/**
	 * @brief Add a dependency rule to the workflow definition.
	 * @param fromId id of the item the rule starts from
	 * @param toId id of the item the rule goes to
	 * @param outcome outcome that satisfies the rule
	 */
	void addDependency(string fromId, string toId, string outcome);

	/**
	 * @brief Set the routing strategy for the work item with this id.
	 * @param itemId id of the work item
	 * @param strategy RoleBasedRouting, DataRuleRouting or ApprovalRouting. The item takes ownership.
	 */
	void setRouting(string itemId, RoutingStrategy *strategy);

	/**
	 * @brief Add a service to the work item with this id.
	 * @param itemId id of the work item
	 * @param serviceName name of the service to attach
	 */
	void setService(string itemId, string serviceName);

	/**
	 * @brief Mark the work item with this id as a checkpoint.
	 * @param itemId id of the item whose checkpoint flag is raised
	 */
	void markCheckpoint(string itemId);

	/**
	 * @brief Wrap the work item with this id in a priority decorator.
	 * @param itemId id of the work item
	 * @param level priority level
	 */
	void addPriority(string itemId, int level);

	/**
	 * @brief Wrap the work item with this id in an audit decorator.
	 * @param itemId id of the work item
	 */
	void addAudit(string itemId);

	/**
	 * @brief Find a work item created during this build.
	 * @param itemId id to look up
	 * @return WorkItem* or nullptr
	 */
	WorkItem *findItem(string itemId);

	/**
	 * @brief Replace the work-item pointer with the replacement.
	 * @param current pointer that is changed
	 * @param replacement work item now stored in that pointer
	 */
	void replaceItem(WorkItem *&current, WorkItem *replacement);
};

#endif
