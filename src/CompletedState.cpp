/**
 * @file CompletedState.cpp
 * @brief Transitions available from the Completed state.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "CompletedState.h"

std::string CompletedState::name() const {
    return "Completed";
}

bool CompletedState::isFinal() const {
    return true;
}

WorkItemState* CompletedState::clone() const {
    return new CompletedState(*this);
}
