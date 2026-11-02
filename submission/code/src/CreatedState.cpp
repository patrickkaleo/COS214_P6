/**
 * @file CreatedState.cpp
 * @brief Transitions available from the Created state.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "CreatedState.h"
#include "WorkItem.h"
#include "AvailableState.h"
#include "CancelledState.h"

bool CreatedState::makeAvailable(WorkItem& item) {
    item.changeState(new AvailableState(), EventType::AVAILABLE);
    return true;
}

bool CreatedState::cancel(WorkItem& item) {
    item.changeState(new CancelledState(), EventType::CANCELLED);
    return true;
}

std::string CreatedState::name() const {
    return "Created";
}

WorkItemState* CreatedState::clone() const {
    return new CreatedState(*this);
}
