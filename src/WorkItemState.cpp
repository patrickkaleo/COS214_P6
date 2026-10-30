/**
 * @file WorkItemState.cpp
 * @brief Default (refusing) behaviour for every work-item lifecycle request.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "WorkItemState.h"

// Each request is refused unless a concrete state overrides it, so a state
// only has to describe the transitions that are legal from it.

bool WorkItemState::makeAvailable(WorkItem& item) {
    (void)item;
    return false;
}

bool WorkItemState::assign(WorkItem& item, Participant* participant) {
    (void)item;
    (void)participant;
    return false;
}

bool WorkItemState::start(WorkItem& item) {
    (void)item;
    return false;
}

bool WorkItemState::complete(WorkItem& item, const std::string& outcome) {
    (void)item;
    (void)outcome;
    return false;
}

bool WorkItemState::reject(WorkItem& item, const std::string& reason) {
    (void)item;
    (void)reason;
    return false;
}

bool WorkItemState::cancel(WorkItem& item) {
    (void)item;
    return false;
}

bool WorkItemState::escalate(WorkItem& item) {
    (void)item;
    return false;
}

bool WorkItemState::isFinal() const {
    return false;
}
