#ifndef AUDITLOGGER_H
#define AUDITLOGGER_H

class AuditLogger : WorkflowEventListener {

public:
	vector<string> records;

	void onEvent(string eventCode, string sourceId);

	const vector<string>& getRecords();
};

#endif
