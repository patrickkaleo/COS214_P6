#ifndef APPROVALHANDLER_H
#define APPROVALHANDLER_H

class ApprovalHandler {

private:
	ApprovalHandler* nextHandler;

public:
	void setNext(ApprovalHandler* next);

	virtual Participant* handle(ApprovalRequest& request) = 0;
};

#endif
