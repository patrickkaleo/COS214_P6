#ifndef ALWAYSACCEPTRULE_H
#define ALWAYSACCEPTRULE_H

class AlwaysAcceptRule : ApprovalRule {


public:
	bool accepts(const ApprovalRequest& request);
};

#endif
