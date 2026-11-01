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

void WorkItemDecorator::changeState(WorkItemState* next, EventType event) {
    if (inner_) inner_->changeState(next, event);
    else WorkItem::changeState(next, event);
}

void WorkItemDecorator::setState(WorkItemState* state) {
    if (inner_) inner_->setState(state);
    else WorkItem::setState(state);
}

WorkItemState* WorkItemDecorator::getState() const {
    return inner_ ? inner_->getState() : WorkItem::getState();
}

void WorkItemDecorator::setAssignee(Participant* participant) {
    if (inner_) inner_->setAssignee(participant);
    else WorkItem::setAssignee(participant);
}

Participant* WorkItemDecorator::getAssignee() const {
    return inner_ ? inner_->getAssignee() : WorkItem::getAssignee();
}

void WorkItemDecorator::setOutcome(const string& outcome) {
    if (inner_) inner_->setOutcome(outcome);
    else WorkItem::setOutcome(outcome);
}

string WorkItemDecorator::getOutcome() const {
    return inner_ ? inner_->getOutcome() : WorkItem::getOutcome();
}

void WorkItemDecorator::incrementEscalation() {
    if (inner_) inner_->incrementEscalation();
    else WorkItem::incrementEscalation();
}

int WorkItemDecorator::getEscalationLevel() const {
    return inner_ ? inner_->getEscalationLevel() : WorkItem::getEscalationLevel();
}

void WorkItemDecorator::setEscalationLevel(int level) {
    if (inner_) inner_->setEscalationLevel(level);
    else WorkItem::setEscalationLevel(level);
}

bool WorkItemDecorator::isFinished() const {
    return inner_ ? inner_->isFinished() : WorkItem::isFinished();
}

string WorkItemDecorator::getStateName() const {
    return inner_ ? inner_->getStateName() : WorkItem::getStateName();
}

string WorkItemDecorator::getInstanceId() const {
    return inner_ ? inner_->getInstanceId() : WorkItem::getInstanceId();
}

void WorkItemDecorator::setInstanceId(const string& id) {
    if (inner_) inner_->setInstanceId(id);
    else WorkItem::setInstanceId(id);
}

SubProcess* WorkItemDecorator::getParent() const {
    return inner_ ? inner_->getParent() : WorkItem::getParent();
}

void WorkItemDecorator::setParent(SubProcess* parent) {
    if (inner_) inner_->setParent(parent);
    else WorkItem::setParent(parent);
}

WorkflowMediator* WorkItemDecorator::getMediator() const {
    return inner_ ? inner_->getMediator() : WorkItem::getMediator();
}

void WorkItemDecorator::setMediator(WorkflowMediator* mediator) {
    if (inner_) inner_->setMediator(mediator);
    else WorkItem::setMediator(mediator);
}

void WorkItemDecorator::attach(WorkflowEventListener* listener) {
    if (inner_) inner_->attach(listener);
    else WorkItem::attach(listener);
}

void WorkItemDecorator::detach(WorkflowEventListener* listener) {
    if (inner_) inner_->detach(listener);
    else WorkItem::detach(listener);
}

void WorkItemDecorator::notify(const WorkflowEvent& event) {
    if (inner_) inner_->notify(event);
    else WorkItem::notify(event);
}

int WorkItemDecorator::childCount() const {
    return inner_ ? inner_->childCount() : 0;
}

WorkItem* WorkItemDecorator::getChild(int index) const {
    return inner_ ? inner_->getChild(index) : nullptr;
}

void WorkItemDecorator::setRoutingContext(const InstanceData* data, const ParticipantDirectory* directory) {
    if (inner_) inner_->setRoutingContext(data, directory);
}

void WorkItemDecorator::onFinished() {
    if (inner_) inner_->onFinished();
}

void WorkItemDecorator::onAvailable() {
    if (inner_) {
        inner_->onAvailable();
    }
}

WorkItem* WorkItemDecorator::getInner() const {
    return inner_;
}

WorkItem* WorkItemDecorator::releaseInner() {
    WorkItem* inner = inner_;
    inner_ = nullptr;
    return inner;
}
