#ifndef WORKFLOWSNAPSHOT_H
#define WORKFLOWSNAPSHOT_H

/** @brief The WorkflowSnapshot class represents a snapshot of a workflow instance at a specific point in time.
 *
 * This class stores the state of a workflow instance, including its processes, data, and participants.
 */
#include <map>
#include <string>
#include <set>
#include <ctime>
#include "InstanceData.h"

/** @brief The WorkflowSnapshot class represents a snapshot of a workflow instance at a specific point in time.
 *
 * This class stores the state of a workflow instance, including its processes, data, and participants.
 */
class WorkflowInstance;

class WorkflowSnapshot
{
	friend class WorkflowInstance;

private:
	std::string label;
	InstanceData data;
	std::map<std::string, std::string> outcomes;
	std::map<std::string, std::string> assignees;
	std::map<std::string, std::string> itemStates;
	std::map<std::string, int> escalationLevels;
	std::set<std::string> openedBranches;
	std::set<std::string> decidedItems;
	std::map<std::string, std::time_t> deadlines;
	std::map<std::string, std::string> deadlineOwners;

public:
	/** @brief Constructs a new WorkflowSnapshot instance.
	 * @param itemStates The states of the work items.
	 * @param assignees The assignees of the work items.
	 * @param outcomes The outcomes of the work items.
	 * @param data The instance data.
	 * @param label The label for the snapshot.
	 */
	WorkflowSnapshot(const std::map<std::string, std::string> &itemStates,
									 const std::map<std::string, std::string> &assignees,
									 const std::map<std::string, std::string> &outcomes,
									 const InstanceData &data,
									 const std::string &label);

	/** @brief Gets the label of the workflow snapshot.
	 * @return The label of the workflow snapshot.
	 */
	std::string getLabel() const;
};
#endif
