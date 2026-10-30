#ifndef WORKFLOWSNAPSHOT_H
#define WORKFLOWSNAPSHOT_H

#include <string>
#include <map>
#include "InstanceData.h"

class WorkflowInstance;

/**
 * @brief Immutable-to-outsiders snapshot of a workflow instance's state.
 *
 * @details Acts as the Memento. Only its friend WorkflowInstance may create
 * a snapshot or read saved internal fields. HistoryLog stores snapshots
 * without accessing their contents.
 */
class WorkflowSnapshot {
    /** @brief Allows the Originator to create and restore snapshots. */
    friend class WorkflowInstance;

private:
    /** @brief Human-readable label identifying the checkpoint. */
    std::string label;
    /** @brief Copy of instance data at capture time. */
    InstanceData data;
    /** @brief Outcome stored for each work-item ID. */
    std::map<std::string, std::string> outcomes;
    /** @brief Assigned participant ID for each assigned work item. */
    std::map<std::string, std::string> assignees;
    /** @brief Lifecycle-state name stored for each work-item ID. */
    std::map<std::string, std::string> itemStates;

    /**
     * @brief Creates a snapshot from the Originator's captured information.
     * @param itemStates Work-item ID to lifecycle-state name mapping.
     * @param assignees Work-item ID to assigned participant ID mapping.
     * @param outcomes Work-item ID to recorded outcome mapping.
     * @param data Captured instance data.
     * @param label Checkpoint label.
     */
    WorkflowSnapshot(const std::map<std::string, std::string>& itemStates,const std::map<std::string, std::string>& assignees,const std::map<std::string, std::string>& outcomes,const InstanceData& data,const std::string& label);

public:
    /**
     * @brief Gets the descriptive name of this checkpoint.
     * @return Snapshot label.
     */
    std::string getLabel() const;
};

#endif
