#ifndef HISTORYLOG_H
#define HISTORYLOG_H
#include<iostream>
#include<map>
#include<vector>
#include "WorkflowSnapshot.h"


class HistoryLog {

private:
	map<string, vector<WorkflowSnapshot*>> snapshots;

public:
	void pushSnapshot(string & InstanceID, WorkflowSnapshot* snapshot);

	WorkflowSnapshot* popSnapshot(const string& instanceId);

	WorkflowSnapshot* latest(const string & InstanceID) const;
	~HistoryLog();
};

#endif
