#ifndef CLAIMACTION_H
#define CLAIMACTION_H

#include "WorkItemAction.h"

class ClaimAction : public WorkItemAction
{
protected:
	bool doExecute(WorkItem &item);

public:
	ClaimAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor);

	std::string describe() const;
};

#endif
