#ifndef CREATEDSTATE_H
#define CREATEDSTATE_H

/**
 * @file CreatedState.h
 * @brief Concrete State: work item is Created.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "WorkItemState.h"

/**
 * @class CreatedState
 * @brief Initial state: the item exists in the instance but its dependencies are not yet met.
 *
 * Legal requests: makeAvailable, cancel.
 */
class CreatedState : public WorkItemState {
public:
    /// Created -> Available.
    bool makeAvailable(WorkItem& item) override;

    /// -> Cancelled.
    bool cancel(WorkItem& item) override;

    std::string name() const override;
    WorkItemState* clone() const override;
};

#endif // CREATEDSTATE_H
