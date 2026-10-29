/**
 * @file WorkItemDecorator.cpp
 * @brief Abstract Decorator for WorkItem (Structural pattern).
 * @details Owns an inner WorkItem and forwards every operation to it.
 *          Concrete decorators add behaviour before or after the forward.
 * @version 1.0
 */

#include "WorkItemDecorator.h"

#include "Participant.h"
#include "WorkflowEvent.h"

#include <iostream>
#include <string>

using namespace std;

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

bool WorkItemDecorator::complete(const string& outcome) {
    return inner_ ? inner_->complete(outcome) : false;
}

bool WorkItemDecorator::reject(const string& reason) {
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