#ifndef ESCALATEDSTATE_H
#define ESCALATEDSTATE_H

/**
 * @file EscalatedState.h
 * @brief Concrete State: work item is Escalated.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "WorkItemState.h"

/**
 * @class EscalatedState
 * @brief The item exceeded a deadline or authority limit and waits for a higher-authority participant.
 *
 * Legal requests: assign, cancel.
 */
class EscalatedState : public WorkItemState {
public:
    /// -> Assigned; refused if @p participant is nullptr.
    bool assign(WorkItem& item, Participant* participant) override;

    /// -> Cancelled.
    bool cancel(WorkItem& item) override;

    std::string name() const override;
    WorkItemState* clone() const override;
};

#endif // ESCALATEDSTATE_H
