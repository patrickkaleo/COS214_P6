#ifndef STARTACTION_H
#define STARTACTION_H

#include "WorkItemAction.h"

class StartAction
{
protected:
	bool doExecute(WorkItem &item);

public:
	StartAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor);

	std::string describe() const;
};

#endif
