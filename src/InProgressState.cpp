/**
 * @file InProgressState.cpp
 * @brief Transitions available from the InProgress state.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "InProgressState.h"
#include "WorkItem.h"
#include "CancelledState.h"
#include "CompletedState.h"
#include "EscalatedState.h"
#include "RejectedState.h"

bool InProgressState::complete(WorkItem& item, const std::string& outcome) {
    item.setOutcome(outcome);
    item.changeState(new CompletedState(), EventType::COMPLETED);
    return true;
}

bool InProgressState::reject(WorkItem& item, const std::string& reason) {
    item.setOutcome(reason);
    item.changeState(new RejectedState(), EventType::REJECTED);
    return true;
}

bool InProgressState::cancel(WorkItem& item) {
    item.changeState(new CancelledState(), EventType::CANCELLED);
    return true;
}

bool InProgressState::escalate(WorkItem& item) {
    item.incrementEscalation();
    item.setAssignee(nullptr);
    item.changeState(new EscalatedState(), EventType::ESCALATED);
    return true;
}

std::string InProgressState::name() const {
    return "InProgress";
}

WorkItemState* InProgressState::clone() const {
    return new InProgressState(*this);
}
