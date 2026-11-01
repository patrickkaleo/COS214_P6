#ifndef ASSIGNEDSTATE_H
#define ASSIGNEDSTATE_H

/**
 * @file AssignedState.h
 * @brief Concrete State: work item is Assigned.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "WorkItemState.h"

/**
 * @class AssignedState
 * @brief A participant is responsible for the item but has not started it. Re-assigning (delegation) stays in this state.
 *
 * Legal requests: assign, start, cancel, escalate.
 */
class AssignedState : public WorkItemState {
public:
    /// -> Assigned; refused if @p participant is nullptr.
    bool assign(WorkItem& item, Participant* participant) override;

    /// Assigned -> InProgress.
    bool start(WorkItem& item) override;

    /// -> Cancelled.
    bool cancel(WorkItem& item) override;

    /// -> Escalated; the current assignee is released.
    bool escalate(WorkItem& item) override;

    std::string name() const override;
    WorkItemState* clone() const override;
};

#endif // ASSIGNEDSTATE_H
