#ifndef INPROGRESSSTATE_H
#define INPROGRESSSTATE_H

/**
 * @file InProgressState.h
 * @brief Concrete State: work item is InProgress.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "WorkItemState.h"

/**
 * @class InProgressState
 * @brief The assignee is working on the item; it can now be completed or rejected.
 *
 * Legal requests: complete, reject, cancel, escalate.
 */
class InProgressState : public WorkItemState {
public:
    /// InProgress -> Completed, recording @p outcome.
    bool complete(WorkItem& item, const std::string& outcome) override;

    /// InProgress -> Rejected, recording @p reason.
    bool reject(WorkItem& item, const std::string& reason) override;

    /// -> Cancelled.
    bool cancel(WorkItem& item) override;

    /// -> Escalated; the current assignee is released.
    bool escalate(WorkItem& item) override;

    std::string name() const override;
    WorkItemState* clone() const override;
};

#endif // INPROGRESSSTATE_H
