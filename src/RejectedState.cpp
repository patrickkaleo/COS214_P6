/**
 * @file RejectedState.cpp
 * @brief Transitions available from the Rejected state.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "RejectedState.h"

std::string RejectedState::name() const {
    return "Rejected";
}

bool RejectedState::isFinal() const {
    return true;
}

WorkItemState* RejectedState::clone() const {
    return new RejectedState(*this);
}
