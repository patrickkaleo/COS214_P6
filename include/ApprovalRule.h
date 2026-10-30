#ifndef APPROVALRULE_H
#define APPROVALRULE_H

class ApprovalRule {


public:
	bool accepts(const ApprovalRequest& request);
};

#endif
