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
#include "WorkItemState.h"

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

WorkItemState* WorkItemDecorator::getState() const {
    return inner_ ? inner_->getState() : nullptr;
}

void WorkItemDecorator::setState(WorkItemState* state) {
    if (inner_) inner_->setState(state);
}

void WorkItemDecorator::setAssignee(Participant* participant) {
    if (inner_) inner_->setAssignee(participant);
}

void WorkItemDecorator::setOutcome(const string& outcome) {
    if (inner_) inner_->setOutcome(outcome);
}

void WorkItemDecorator::incrementEscalation() {
    if (inner_) inner_->incrementEscalation();
}

bool WorkItemDecorator::isFinished() const {
    return inner_ ? inner_->isFinished() : false;
}

string WorkItemDecorator::getStateName() const {
    return inner_ ? inner_->getStateName() : "";
}

string WorkItemDecorator::getOutcome() const {
    return inner_ ? inner_->getOutcome() : "";
}

Participant* WorkItemDecorator::getAssignee() const {
    return inner_ ? inner_->getAssignee() : nullptr;
}

int WorkItemDecorator::getEscalationLevel() const {
    return inner_ ? inner_->getEscalationLevel() : 0;
}

void WorkItemDecorator::setEscalationLevel(int level) {
    if (inner_) inner_->setEscalationLevel(level);
}

string WorkItemDecorator::getInstanceId() const {
    return inner_ ? inner_->getInstanceId() : "";
}

void WorkItemDecorator::setInstanceId(const string& id) {
    if (inner_) inner_->setInstanceId(id);
}

SubProcess* WorkItemDecorator::getParent() const {
    return inner_ ? inner_->getParent() : nullptr;
}

void WorkItemDecorator::setParent(SubProcess* parent) {
    if (inner_) inner_->setParent(parent);
}

void WorkItemDecorator::attach(WorkflowEventListener* listener) {
    if (inner_) inner_->attach(listener);
}

void WorkItemDecorator::detach(WorkflowEventListener* listener) {
    if (inner_) inner_->detach(listener);
}

void WorkItemDecorator::setMediator(WorkflowMediator* mediator) {
    if (inner_) inner_->setMediator(mediator);
}

WorkflowMediator* WorkItemDecorator::getMediator() const {
    return inner_ ? inner_->getMediator() : nullptr;
}

int WorkItemDecorator::childCount() const {
    return inner_ ? inner_->childCount() : 0;
}

WorkItem* WorkItemDecorator::getChild(int index) const {
    return inner_ ? inner_->getChild(index) : nullptr;
}

WorkItem* WorkItemDecorator::undecorated() {
    return inner_ ? inner_->undecorated() : this;
}

int WorkItemDecorator::getPriority() const {
    return inner_ ? inner_->getPriority() : 0;
}

void WorkItemDecorator::setRoutingContext(const InstanceData* data, const ParticipantDirectory* directory) {
    if (inner_) inner_->setRoutingContext(data, directory);
}
