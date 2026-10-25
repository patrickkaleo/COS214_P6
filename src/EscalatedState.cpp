/**
 * @file EscalatedState.cpp
 * @brief Transitions available from the Escalated state.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "EscalatedState.h"
#include "WorkItem.h"
#include "AssignedState.h"
#include "CancelledState.h"

bool EscalatedState::assign(WorkItem& item, Participant* participant) {
    if (participant == nullptr) {
        return false;
    }
    item.setAssignee(participant);
    item.changeState(new AssignedState(), EventType::ASSIGNED);
    return true;
}

bool EscalatedState::cancel(WorkItem& item) {
    item.changeState(new CancelledState(), EventType::CANCELLED);
    return true;
}

std::string EscalatedState::name() const {
    return "Escalated";
}

WorkItemState* EscalatedState::clone() const {
    return new EscalatedState(*this);
}
