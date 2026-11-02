#ifndef WORKITEMACTION_H
#define WORKITEMACTION_H

/** @brief The WorkItemAction class represents an action that can be performed on a work item.
 *
 * This class provides a common interface for different types of actions that can be applied to work items within a workflow instance.
 */
#include <string>

class WorkflowSnapshot;
class WorkflowInstance;
class WorkItem;
class Participant;

class WorkItemAction
{

protected:
	WorkflowInstance *instance;
	std::string itemId;
	Participant *actor;
	WorkflowSnapshot *before;
	std::string operation;

	/** @brief Executes the action on the specified work item.
	 * @param item The work item to execute the action on.
	 * @return True if the action was successful, false otherwise.
	 */
	virtual bool doExecute(WorkItem &item) = 0;
	/** @brief Checks if the actor is authorized to perform the action on the specified work item.
	 * @param item The work item to check authorization for.
	 * @return True if the actor is authorized, false otherwise.
	 */
	virtual bool isAuthorized(const WorkItem &item) const;
	/** @brief Generates a label for the workflow snapshot based on the work item.
	 * @param item The work item to generate a label for.
	 * @return The generated label.
	 */
	virtual std::string snapshotLabel(const WorkItem &item) const;

public:
	/** @brief Constructs a new WorkItemAction instance.
	 * @param instance The workflow instance.
	 * @param itemId The ID of the work item.
	 * @param actor The participant performing the action.
	 * @param operation The operation to perform.
	 */
	WorkItemAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor,
								 const std::string &operation = "");

	/** @brief Virtual destructor for the WorkItemAction class. */
	virtual ~WorkItemAction();
	/** @brief Executes the action on the specified work item.
	 * @param item The work item to execute the action on.
	 * @return True if the action was successful, false otherwise.
	 */
	bool execute();
	/** @brief Undoes the action on the specified work item.
	 * @param item The work item to undo the action on.
	 * @return True if the action was successfully undone, false otherwise.
	 */
	bool undo();
	/** @brief Gets the workflow instance.
	 * @return The workflow instance.
	 */
	WorkflowInstance *getInstance() const { return instance; }
	/** @brief Gets the label for the workflow snapshot.
	 * @return The snapshot label.
	 */
	std::string getSnapshotLabel() const;
	/** @brief Describes the action.
	 * @return A string describing the action.
	 */
	virtual std::string describe() const = 0;
	/** @brief Gets the description of the action.
	 * @return The description of the action.
	 */
	void description();
};

#endif
