#ifndef REJECTEDSTATE_H
#define REJECTEDSTATE_H

/**
 * @file RejectedState.h
 * @brief Concrete State: work item is Rejected.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "WorkItemState.h"

/**
 * @class RejectedState
 * @brief Final state: the item finished unsuccessfully; the reason is kept as its outcome.
 *
 * Legal requests: none (final state); every request is refused.
 */
class RejectedState : public WorkItemState {
public:
    std::string name() const override;
    bool isFinal() const override;
    WorkItemState* clone() const override;
};

#endif // REJECTEDSTATE_H
