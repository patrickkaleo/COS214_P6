#ifndef DELEGATEACTION_H
#define DELEGATEACTION_H

#include "WorkItemAction.h"

class DelegateAction : public WorkItemAction
{

private:
	Participant *to;

public:
	DelegateAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor, Participant *to);
	bool doExecute(WorkItem &item);
	std::string describe() const;
};

#endif
