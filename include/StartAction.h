#ifndef STARTACTION_H
#define STARTACTION_H

#include "WorkItemAction.h"

class StartAction : public WorkItemAction {
protected:
    bool doExecute(WorkItem& item) override;

public:
    StartAction(WorkflowInstance* instance,
                const std::string& itemId,
                Participant* actor);

    std::string describe() const override;
};

#endif
