#ifndef CONCRETEAPPROVALHANDLER_H
#define CONCRETEAPPROVALHANDLER_H

class ConcreteApprovalHandler : ApprovalHandler {

private:
	string approverRole;
	ApprovalRule* rule;
	int deadlineHours;

public:
	Participant* handle(ApprovalRequest& request);
};

#endif
