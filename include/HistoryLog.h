
#ifndef HISTORYLOG_H
#define HISTORYLOG_H

#include <string>
#include <map>
#include <vector>
#include "WorkflowSnapshot.h"

class HistoryLog
{
private:
	std::map<std::string, std::vector<WorkflowSnapshot *>> snapshots;

public:
	HistoryLog() = default;
	~HistoryLog();

	// Store a snapshot for a specific workflow instance
	void pushSnapshots(const std::string &instanceID,
										 WorkflowSnapshot *snapshot);

	// Remove and return the newest snapshot
	WorkflowSnapshot *popSnapshots(const std::string &instanceID);

	// View the newest snapshot without removing it
	const WorkflowSnapshot *latest(const std::string &instanceID) const;
};

#endif
