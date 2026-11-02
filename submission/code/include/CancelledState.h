#ifndef CANCELLEDSTATE_H
#define CANCELLEDSTATE_H

/**
 * @file CancelledState.h
 * @brief Concrete State: work item is Cancelled.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "WorkItemState.h"

/**
 * @class CancelledState
 * @brief Final state: the item was aborted or is no longer needed.
 *
 * Legal requests: none (final state); every request is refused.
 */
class CancelledState : public WorkItemState {
public:
    std::string name() const override;
    bool isFinal() const override;
    WorkItemState* clone() const override;
};

#endif // CANCELLEDSTATE_H
