#ifndef AVAILABLESTATE_H
#define AVAILABLESTATE_H

/**
 * @file AvailableState.h
 * @brief Concrete State: work item is Available.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "WorkItemState.h"

/**
 * @class AvailableState
 * @brief The item's dependencies are met and it is waiting for a participant.
 *
 * Legal requests: assign, cancel, escalate.
 */
class AvailableState : public WorkItemState {
public:
    /// -> Assigned; refused if @p participant is nullptr.
    bool assign(WorkItem& item, Participant* participant) override;

    /// -> Cancelled.
    bool cancel(WorkItem& item) override;

    /// -> Escalated; the current assignee is released.
    bool escalate(WorkItem& item) override;

    std::string name() const override;
    WorkItemState* clone() const override;
};

#endif // AVAILABLESTATE_H
