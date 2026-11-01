
#ifndef HISTORYLOG_H
#define HISTORYLOG_H

/**
 * @file HistoryLog.h
 * @author Ntando Dlamini
 * @brief
 * @version 0.1
 * @date 2026-11-01
 *
 * @copyright Copyright (c) 2026
 *
 */

#include <string>
#include <map>
#include <vector>
#include "WorkflowSnapshot.h"

/**
 * @brief The HistoryLog class manages a log of workflow snapshots.
 */
class HistoryLog
{
private:
	std::map<std::string, std::vector<WorkflowSnapshot *>> snapshots;

public:
	HistoryLog() = default;
	~HistoryLog();

	/** @brief Stores a snapshot for a specific workflow instance.
	 * @param instanceID The ID of the workflow instance.
	 * @param snapshot The snapshot to store.
	 */
	void pushSnapshots(const std::string &instanceID,
										 WorkflowSnapshot *snapshot);

	/** @brief Removes and returns the newest snapshot for a specific workflow instance.
	 * @param instanceID The ID of the workflow instance.
	 * @return A pointer to the removed snapshot, or nullptr if no snapshot is available.
	 */
	WorkflowSnapshot *popSnapshots(const std::string &instanceID);

	/** @brief Views the newest snapshot for a specific workflow instance without removing it.
	 * @param instanceID The ID of the workflow instance.
	 * @return A pointer to the latest snapshot, or nullptr if no snapshot is available.
	 */
	const WorkflowSnapshot *latest(const std::string &instanceID) const;
};

#endif
