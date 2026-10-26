/**
 * @file decorator.cpp
 * @brief Implementation of the Decorator hierarchy only:
 *        WorkItemDecorator, PriorityDecorator, AuditDecorator.
 * @version 1.0
 */

#include "WorkItemDecorator.h"
#include "PriorityDecorator.h"
#include "AuditDecorator.h"

#include "AuditLogger.h"
#include "Participant.h"
#include "WorkflowEvent.h"

#include <iostream>
#include <string>

// WorkItemDecorator
WorkItemDecorator::WorkItemDecorator(WorkItem* inner)
    : WorkItem(inner ? inner->getId() : "unknown", inner ? inner->getName() : "unknown"),
      inner_(inner) // Adopt the inner item's identity so the rest of the engine still sees the original id/name
{}

WorkItemDecorator::~WorkItemDecorator() {
    delete inner_;
}

bool WorkItemDecorator::makeAvailable() {
    return inner_ ? inner_->makeAvailable() : false;
}

bool WorkItemDecorator::assign(Participant* participant) {
    return inner_ ? inner_->assign(participant) : false;
}

bool WorkItemDecorator::start() {
    return inner_ ? inner_->start() : false;
}

bool WorkItemDecorator::complete(const std::string& outcome) {
    return inner_ ? inner_->complete(outcome) : false;
}

bool WorkItemDecorator::reject(const std::string& reason) {
    return inner_ ? inner_->reject(reason) : false;
}

bool WorkItemDecorator::cancel() {
    return inner_ ? inner_->cancel() : false;
}

bool WorkItemDecorator::escalate() {
    return inner_ ? inner_->escalate() : false;
}

void WorkItemDecorator::onAvailable() {
    if (inner_) {
        inner_->onAvailable();
    }
}

WorkItem* WorkItemDecorator::getInner() const {
    return inner_;
}

// PriorityDecorator
PriorityDecorator::PriorityDecorator(WorkItem* inner, int level)
    : WorkItemDecorator(inner),
      level_(level)
{}

int PriorityDecorator::getPriority() const {
    return level_;
}

void PriorityDecorator::onAvailable() {
    // Extra behaviour first
    std::cout << "[PriorityDecorator] \"" << getName() << "\" flagged as priority level " << level_ << "\n";
    // Then forward
    WorkItemDecorator::onAvailable();
}

WorkItem* PriorityDecorator::clone() const {
    WorkItem* innerClone = inner_ ? inner_->clone() : nullptr;
    return new PriorityDecorator(innerClone, level_);
}

// AuditDecorator
AuditDecorator::AuditDecorator(WorkItem* inner, AuditLogger* logger)
    : WorkItemDecorator(inner),
      logger_(logger)
{}

void AuditDecorator::audit(const std::string& action) const {
    if (logger_) {
        // Real AuditLogger API
        std::cout << "[AuditDecorator] " << action << " on \"" << getName() << "\" (id=" << getId() << ")\n";
        // logger_->onEvent(WorkflowEvent{EventType::..., ..., id_, name_, ...});
    }
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

bool AuditDecorator::complete(const std::string& outcome) {
    audit("complete(outcome=" + outcome + ")");
    return WorkItemDecorator::complete(outcome);
}

bool AuditDecorator::reject(const std::string& reason) {
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
