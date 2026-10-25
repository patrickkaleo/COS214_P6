/**
 * @file AvailableState.cpp
 * @brief Transitions available from the Available state.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "AvailableState.h"
#include "WorkItem.h"
#include "AssignedState.h"
#include "CancelledState.h"
#include "EscalatedState.h"

bool AvailableState::assign(WorkItem& item, Participant* participant) {
    if (participant == nullptr) {
        return false;
    }
    item.setAssignee(participant);
    item.changeState(new AssignedState(), EventType::ASSIGNED);
    return true;
}

bool AvailableState::cancel(WorkItem& item) {
    item.changeState(new CancelledState(), EventType::CANCELLED);
    return true;
}

bool AvailableState::escalate(WorkItem& item) {
    item.incrementEscalation();
    item.setAssignee(nullptr);
    item.changeState(new EscalatedState(), EventType::ESCALATED);
    return true;
}

std::string AvailableState::name() const {
    return "Available";
}

WorkItemState* AvailableState::clone() const {
    return new AvailableState(*this);
}
