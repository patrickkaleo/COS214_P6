#ifndef COMPLETEACTION_H
#define COMPLETEACTION_H

#include "WorkItemAction.h"

class CompleteAction : WorkItemAction
{

private:
	std::string outcome;

protected:
	bool doExecute(WorkItem &item);

public:
	CompleteAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor, const std::string &outcome);

	std::string describe() const;
};

#endif
