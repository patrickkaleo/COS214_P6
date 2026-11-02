/**
 * @file CancelledState.cpp
 * @brief Transitions available from the Cancelled state.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "CancelledState.h"

std::string CancelledState::name() const {
    return "Cancelled";
}

bool CancelledState::isFinal() const {
    return true;
}

WorkItemState* CancelledState::clone() const {
    return new CancelledState(*this);
}
