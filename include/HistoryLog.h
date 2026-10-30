
#ifndef HISTORYLOG_H
#define HISTORYLOG_H

#include <string>
#include <vector>
#include <map>
#include "WorkflowSnapshot.h"

/**
 * @brief Stores recovery checkpoints for workflow instances.
 *
 * @details Acts as the Caretaker in the Memento pattern. Each instance ID
 * maps to a stack of snapshots. The log owns snapshots while they remain
 * in the collection but does not inspect their saved internal state.
 */
class HistoryLog {
private:
    /** @brief Owned snapshots grouped by workflow instance ID. */
    std::map<std::string, std::vector<WorkflowSnapshot*>> snapshots;

public:
    /** @brief Deletes snapshots still owned by the history log. */
    ~HistoryLog();

    /**
     * @brief Adds a snapshot to an instance's checkpoint history.
     * @param instanceId Identifier of the workflow instance.
     * @param snapshot Snapshot pointer whose ownership transfers to the log.
     * @note A null snapshot is ignored by the current implementation.
     */
    void pushSnapshots(const std::string& instanceId, WorkflowSnapshot* snapshot);

    /**
     * @brief Removes and returns the newest snapshot for an instance.
     * @param instanceId Identifier of the workflow instance.
     * @return Removed snapshot, or nullptr if none exists.
     * @note Ownership of the returned snapshot passes to the caller.
     */
    WorkflowSnapshot* popSnapshots(const std::string& instanceId);

    /**
     * @brief Views an instance's most recent snapshot without removing it.
     * @param instanceId Identifier of the workflow instance.
     * @return Non-owning read-only snapshot pointer, or nullptr if absent.
     */
    const WorkflowSnapshot* latest(const std::string& instanceId) const;
};

#endif


