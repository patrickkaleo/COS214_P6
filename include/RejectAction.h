#ifndef REJECTACTION_H
#define REJECTACTION_H

#include "WorkItemAction.h"

class RejectAction : public WorkItemAction
{

private:
	std::string reason;

protected:
	bool doExecute(WorkItem &item);

public:
	RejectAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor, const std::string &reason);
	std::string describe() const;
};

#endif
