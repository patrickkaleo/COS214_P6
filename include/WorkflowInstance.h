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

/** @brief The WorkflowInstance class represents a running instance of a workflow definition.
 *
 * This class manages the execution state of a workflow instance, including its processes, data, and participants.
 */
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
	WorkItem *rootProcess;								// owned, including any decorator around the root
	InstanceData data;
	ParticipantDirectory *participants; // not owned
	std::map<std::string, std::time_t> deadlines;
	std::map<std::string, std::string> deadlineOwners;
	/** @brief Wires the runtime components for the workflow instance.
	 * @param services The service registry to use.
	 */
	void wireRuntime(ServiceRegistry *services);
	/** @brief Finds a work item recursively within the workflow instance.
	 * @param current The current work item to search from.
	 * @param itemId The ID of the item to find.
	 * @return The found work item, or nullptr if not found.
	 */
	WorkItem *findItemRecursive(WorkItem *current, const std::string &itemId) const;
	/** @brief Captures the state of a work item.
	 * @param item The work item to capture.
	 * @param states The map to store the item's states.
	 * @param assignees The map to store the item's assignees.
	 * @param outcomes The map to store the item's outcomes.
	 */
	void captureItem(WorkItem *item,
									 std::map<std::string, std::string> &states,
									 std::map<std::string, std::string> &assignees,
									 std::map<std::string, std::string> &outcomes) const;
	/** @brief Restores the state of a work item.
	 * @param item The work item to restore.
	 */
	void restoreItem(WorkItem *item);
	/** @brief Clears the assignees of a work item.
	 * @param item The work item to clear assignees for.
	 */
	void clearAssignees(WorkItem *item);
	/** @brief Creates a work item state from a name.
	 * @param stateName The name of the state to create.
	 * @return The created work item state.
	 */
	WorkItemState *createStateFromName(const std::string &stateName) const;

public:
	/** @brief Constructs a new WorkflowInstance instance.
	 * @param id The ID of the workflow instance.
	 * @param definition The workflow definition.
	 * @param data The instance data.
	 */
	WorkflowInstance(const std::string &id,
									 const WorkflowDefinition *definition,
									 const InstanceData &data);

	/** @brief Constructs a new WorkflowInstance instance with additional parameters.
	 * @param id The ID of the workflow instance.
	 * @param definition The workflow definition.
	 * @param data The instance data.
	 * @param directory The participant directory.
	 * @param services The service registry.
	 */
	WorkflowInstance(const std::string &id, const WorkflowDefinition *definition,
									 const InstanceData &data, ParticipantDirectory *directory,
									 ServiceRegistry *services);

	/** @brief Constructs a new WorkflowInstance instance with a root sub-process.
	 * @param id The ID of the workflow instance.
	 * @param definition The workflow definition.
	 * @param root The root sub-process.
	 * @param mediator The dependency mediator.
	 * @param data The instance data.
	 * @param directory The participant directory.
	 */
	WorkflowInstance(const std::string &id, const WorkflowDefinition *definition,
									 SubProcess *root, DependencyMediator *mediator,
									 const InstanceData &data, ParticipantDirectory *directory);
	~WorkflowInstance();
	WorkflowInstance(const WorkflowInstance &) = delete;
	WorkflowInstance &operator=(const WorkflowInstance &) = delete;

	/** @brief Starts the workflow instance.
	 * @return True if the workflow instance was started successfully, false otherwise.
	 */
	bool start();

	// Lifecycle operations
	/** @brief Gets the ID of the workflow instance.
	 * @return The ID of the workflow instance.
	 */
	const std::string &getId() const;
	/** @brief Finds a work item by its ID.
	 * @param itemId The ID of the work item to find.
	 * @return The found work item, or nullptr if not found.
	 */
	WorkItem *findItem(const std::string &itemId) const;
	/** @brief Gets the instance data.
	 * @return A reference to the instance data.
	 */
	InstanceData &getData();
	/** @brief Gets the instance data (const).
	 * @return A const reference to the instance data.
	 */
	const InstanceData &getData() const;
	/** @brief Gets the root sub-process.
	 * @return The root sub-process.
	 */
	WorkItem *getRoot() const;
	/** @brief Gets the dependency mediator.
	 * @return The dependency mediator.
	 */
	DependencyMediator *getMediator() const;
	/** @brief Gets the participant directory.
	 * @return The participant directory.
	 */
	std::string getDefinitionId() const;
	/** @brief Gets the participant directory.
	 * @return The participant directory.
	 */
	bool isRegisteredParticipant(const Participant *actor) const;
	/** @brief Checks if a participant can be assigned to a work item.
	 * @param item The work item.
	 * @param actor The participant.
	 * @return True if the participant can be assigned, false otherwise.
	 */
	bool canAssign(const WorkItem &item, const Participant *actor) const;
	/** @brief Checks if a participant can perform an action on a work item.
	 * @param item The work item.
	 * @param actor The participant.
	 * @param action The action to perform.
	 * @return True if the participant can perform the action, false otherwise.
	 */
	bool canAct(const WorkItem &item, const Participant *actor,
							const std::string &action) const;

	/** @brief Gets the list of available actions for a work item and participant.
	 * @param itemId The ID of the work item.
	 * @param actor The participant.
	 * @return A vector of available action names.
	 */
	std::vector<std::string> availableActions(const std::string &itemId,
																						const Participant *actor) const;
	/** @brief Refreshes the deadlines for all work items.
	 * @param now The current time.
	 */
	void refreshDeadlines(std::time_t now);
	/** @brief Checks the deadlines for all work items.
	 * @param now The current time.
	 */
	void checkDeadlines(std::time_t now);
	/** @brief Gets the deadline for a work item.
	 * @param itemId The ID of the work item.
	 * @return The deadline for the work item.
	 */
	std::time_t getDeadline(const std::string &itemId) const;
	/** @brief Gets the status of the workflow instance.
	 * @return The status of the workflow instance.
	 */
	std::string status() const;
	/** @brief Creates a snapshot of the workflow instance.
	 * @param label The label for the snapshot.
	 * @return A pointer to the created snapshot.
	 */
	WorkflowSnapshot *createSnapshot(const std::string &label);
	/** @brief Restores the workflow instance from a snapshot.
	 * @param snapshot The snapshot to restore from.
	 */
	void restoreSnapshot(const WorkflowSnapshot &snapshot);
};
#endif
