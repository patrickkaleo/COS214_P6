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

    // instanceId is empty here (WorkItem does not own instance id) Listeners still get item id/name/outcome for the audit trail.
    WorkflowEvent event(type, /*instanceId*/ "", getId(), getName(), outcome);
    logger_->onEvent(event);
}

bool AuditDecorator::makeAvailable() {
    audit("makeAvailable");
    return WorkItemDecorator::makeAvailable();
}

bool AuditDecorator::assign(Participant* participant) {
    audit("assign");
    return WorkItemDecorator::assign(participant);
}

bool AuditDecorator::start() {
    audit("start");
    return WorkItemDecorator::start();
}

bool AuditDecorator::complete(const string& outcome) {
    audit("complete(outcome=" + outcome + ")");
    return WorkItemDecorator::complete(outcome);
}

bool AuditDecorator::reject(const string& reason) {
    audit("reject(reason=" + reason + ")");
    return WorkItemDecorator::reject(reason);
}

bool AuditDecorator::cancel() {
    audit("cancel");
    return WorkItemDecorator::cancel();
}

bool AuditDecorator::escalate() {
    audit("escalate");
    return WorkItemDecorator::escalate();
}

void AuditDecorator::onAvailable() {
    audit("onAvailable");
    WorkItemDecorator::onAvailable();
}

WorkItem* AuditDecorator::clone() const {
    WorkItem* innerClone = inner_ ? inner_->clone() : nullptr;
    // logger_ is shared (owned by the engine) so the clone re-uses the same pointer
    return new AuditDecorator(innerClone, logger_);
}
