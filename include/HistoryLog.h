#ifndef HISTORYLOG_H
#define HISTORYLOG_H

class HistoryLog {

private:
	map<string, vector<WorkflowSnapshot*>> snapshots;

public:
	void pushSnapshot(string InstanceID, WorkflowSnapshot* snapshot);

	WorkflowSnapshot* popSnapshot();

	WorkFlowSnapshot* latest(string InstanceID);
};

#endif
