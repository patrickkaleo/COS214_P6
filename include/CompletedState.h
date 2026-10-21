#ifndef COMPLETEDSTATE_H
#define COMPLETEDSTATE_H

/**
 * @file CompletedState.h
 * @brief Concrete State: work item is Completed.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "WorkItemState.h"

/**
 * @class CompletedState
 * @brief Final state: the item finished successfully with an outcome.
 *
 * Legal requests: none (final state); every request is refused.
 */
class CompletedState : public WorkItemState {
public:
    std::string name() const override;
    bool isFinal() const override;
    WorkItemState* clone() const override;
};

#endif // COMPLETEDSTATE_H
