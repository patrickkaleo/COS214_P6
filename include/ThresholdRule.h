#ifndef THRESHOLDRULE_H
#define THRESHOLDRULE_H

class ThresholdRule : ApprovalRule {

private:
	string field;
	double mavValue;

public:
	bool accepts(const ApprovalRequest& request);
};

#endif
