#ifndef WORKFLOWINSTANCE_H
#define WORKFLOWINSTANCE_H

#include <string>
#include <map>

#include "WorkflowDefinition.h"
#include "DependencyMediator.h"
#include "SubProcess.h"
#include "InstanceData.h"
#include "WorkflowSnapshot.h"
#include "ParticipantDirectory.h"
#include "WorkItemState.h"

class WorkItem;

/**
 * @brief Represents one running instance of a workflow definition.
 *
 * @details Acts as the Originator in the Memento pattern. It owns its
 * runtime work-item tree and dependency mediator, and can capture or
 * restore the instance's data, item states, outcomes, and assignments.
 */
class WorkflowInstance {
private:
    /** @brief Unique identifier of this workflow instance. */
    std::string Id;

    /** @brief Definition used to create this instance; not owned. */
    const WorkflowDefinition* definition;
    /** @brief Root of this instance's work-item tree; owned. */
    SubProcess* rootProcess;
    /** @brief Coordinates dependencies among work items; owned. */
    DependencyMediator* mediator;
    /** @brief Data associated with this running instance. */
    InstanceData data;
    /** @brief Directory used to resolve saved assignee IDs; not owned. */
    ParticipantDirectory* participants;

    /**
     * @brief Searches the work-item tree recursively for an ID.
     * @param current Current work item to inspect.
     * @param itemId ID of the requested work item.
     * @return Pointer to the matching item, or nullptr if absent.
     */
    WorkItem* findItemRecursive(WorkItem* current, const std::string& itemId) const;

    /**
     * @brief Saves an item's state, outcome, assignment, and child data.
     * @param item Current work item to capture.
     * @param states Output map from item ID to lifecycle state name.
     * @param assignees Output map from item ID to participant ID.
     * @param outcomes Output map from item ID to outcome value.
     */
    void captureItem(WorkItem* item,std::map<std::string, std::string>& states,std::map<std::string, std::string>& assignees,std::map<std::string, std::string>& outcomes) const;

    /**
     * @brief Clears task assignees throughout the supplied subtree.
     * @param item Root item of the subtree to clear.
     */
    void clearAssignees(WorkItem* item);

    /**
     * @brief Constructs a concrete lifecycle-state object from its name.
     * @param stateName Saved name of a workflow lifecycle state.
     * @return Newly allocated state, or nullptr for an unknown name.
     * @note The caller is responsible for transferring or releasing ownership.
     */
    WorkItemState* createStateFromName(const std::string& stateName) const;


public:
    /**
     * @brief Constructs a running workflow instance.
     * @param id Unique instance identifier.
     * @param definition Source workflow definition; not owned.
     * @param root Work-item tree root; ownership is transferred.
     * @param mediator Dependency mediator; ownership is transferred.
     * @param data Initial instance data, copied into the instance.
     * @param directory Participant directory; not owned.
     */
    WorkflowInstance(const std::string& id,const WorkflowDefinition* definition,SubProcess* root,DependencyMediator* mediator,const InstanceData& data,ParticipantDirectory* directory);

    /** @brief Destroys the owned root work-item tree and mediator. */
    ~WorkflowInstance();

    /**
     * @brief Makes the root work item available to start execution.
     * @return True if the root accepts the transition; otherwise false.
     */
    bool start();

    /** @brief Returns this instance's unique identifier. */
    const std::string& getId() const;

    /** @brief Returns a modifiable reference to instance data. */
    InstanceData& getData();

    /**
     * @brief Finds a work item by ID in the runtime tree.
     * @param itemId ID to find.
     * @return Non-owning pointer to the item, or nullptr if not found.
     */
    WorkItem* findItem(const std::string& itemId) const;

    /**
     * @brief Reports the instance status based on its root lifecycle state.
     * @return Status string such as running, completed, or terminated.
     */
    std::string status() const;

    /**
     * @brief Captures the current instance in a new Memento object.
     * @param label Human-readable snapshot label.
     * @return Newly allocated snapshot; caller must arrange ownership.
     */
    WorkflowSnapshot* createSnapshot(const std::string& label);

    /**
     * @brief Restores instance data and work-item state from a snapshot.
     * @param snapshot Snapshot containing the saved instance information.
     * @note Restoration should not trigger ordinary lifecycle side effects.
     */
    void restoreSnapshot(const WorkflowSnapshot& snapshot);
};

#endif
