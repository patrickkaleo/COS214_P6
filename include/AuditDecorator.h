#ifndef AUDITDECORATOR_H
#define AUDITDECORATOR_H

class AuditDecorator : WorkItemDecorator {

public:
	AuditLogger* logger;

	bool makeAvailable();

	bool assign(Participant* participant);

	bool start();

	bool complete(string outcome);

	bool reject(string reason);

	bool cancel();

	bool escalate();

	WorkItem* clone();
};

#endif
