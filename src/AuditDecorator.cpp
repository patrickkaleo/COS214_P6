/**
 * @file AuditDecorator.cpp
 * @brief Concrete Decorator that records a detailed audit trail around every
 *        lifecycle operation on a WorkItem.
 * @version 1.0
 */

#include "WorkItemDecorator.h"
#include "AuditDecorator.h"

#include "AuditLogger.h"
#include "Participant.h"
#include "WorkflowEvent.h"

#include <iostream>
#include <string>

using namespace std;

AuditDecorator::AuditDecorator(WorkItem* inner, AuditLogger* logger)
    : WorkItemDecorator(inner),
      logger_(logger)
{}

void AuditDecorator::audit(const string& action) const {
    if (!logger_) return;

    EventType type = EventType::STARTED; //default
    string outcome;

    if (action == "makeAvailable" || action == "onAvailable") {
        type = EventType::AVAILABLE;
    } else if (action == "assign") {
        type = EventType::ASSIGNED;
    } else if (action == "start") {
        type = EventType::STARTED;
    } else if (action.find("complete") == 0) {
        type = EventType::COMPLETED;
        // action may be "complete(outcome=paid)" so extracting after '=' if present
        string::size_type pos = action.find('=');
        if (pos != string::npos && pos + 1 < action.size()) {
            outcome = action.substr(pos + 1);
            if (!outcome.empty() && outcome.back() == ')') {
                outcome.pop_back();
            }
        }
    } else if (action.find("reject") == 0) {
        type = EventType::REJECTED;
        string::size_type pos = action.find('=');
        if (pos != string::npos && pos + 1 < action.size()) {
            outcome = action.substr(pos + 1);
            if (!outcome.empty() && outcome.back() == ')') {
                outcome.pop_back();
            }
        }
    } else if (action == "cancel") {
        type = EventType::CANCELLED;
    } else if (action == "escalate") {
        type = EventType::ESCALATED;
    }

    WorkflowEvent event(type, getInstanceId(), getId(), getName(), outcome);
    logger_->onEvent(event);
}

bool AuditDecorator::makeAvailable() {
    if (!WorkItemDecorator::makeAvailable()) return false;
    audit("makeAvailable");
    return true;
}

bool AuditDecorator::assign(Participant* participant) {
    if (!WorkItemDecorator::assign(participant)) return false;
    audit("assign");
    return true;
}

bool AuditDecorator::start() {
    if (!WorkItemDecorator::start()) return false;
    audit("start");
    return true;
}

bool AuditDecorator::complete(const string& outcome) {
    if (!WorkItemDecorator::complete(outcome)) return false;
    audit("complete(outcome=" + outcome + ")");
    return true;
}

bool AuditDecorator::reject(const string& reason) {
    if (!WorkItemDecorator::reject(reason)) return false;
    audit("reject(reason=" + reason + ")");
    return true;
}

bool AuditDecorator::cancel() {
    if (!WorkItemDecorator::cancel()) return false;
    audit("cancel");
    return true;
}

bool AuditDecorator::escalate() {
    if (!WorkItemDecorator::escalate()) return false;
    audit("escalate");
    return true;
}

void AuditDecorator::onAvailable() {
    WorkItemDecorator::onAvailable();
}

WorkItem* AuditDecorator::clone() const {
    WorkItem* innerClone = inner_ ? inner_->clone() : nullptr;
    // logger_ is shared (owned by the engine) so the clone re-uses the same pointer
    return new AuditDecorator(innerClone, logger_);
}
