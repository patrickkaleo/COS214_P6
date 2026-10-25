/**
 * @file WorkItem.cpp
 * @brief WorkItem lifecycle delegation to WorkItemState and event publication.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "WorkItem.h"
#include "WorkItemState.h"
#include "CreatedState.h"
#include "EventDispatcher.h"
#include "WorkflowMediator.h"

WorkItem::WorkItem(const std::string& id, const std::string& itemName)
    : itemID(id),
      name(itemName),
      instanceId(""),
      outcome(""),
      state(new CreatedState()),
      assignee(nullptr),
      escalationLevel(0),
      parent(nullptr),
      mediator(nullptr) {
}

WorkItem::WorkItem(const WorkItem& other)
    : itemID(other.itemID),
      name(other.name),
      instanceId(other.instanceId),
      outcome(other.outcome),
      state(other.state->clone()),
      assignee(other.assignee),
      escalationLevel(other.escalationLevel),
      parent(nullptr),     // the copying composite re-links its children
      mediator(nullptr) {  // each instance registers its own copy with its own mediator
}

WorkItem::~WorkItem() = default;

bool WorkItem::makeAvailable() {
    if (!state->makeAvailable(*this)) {
        return false;
    }
    onAvailable();
    return true;
}

bool WorkItem::assign(Participant* participant) {
    return state->assign(*this, participant);
}

bool WorkItem::start() {
    return state->start(*this);
}

bool WorkItem::complete(const std::string& result) {
    if (!state->complete(*this, result)) {
        return false;
    }
    finished();
    return true;
}

bool WorkItem::reject(const std::string& reason) {
    if (!state->reject(*this, reason)) {
        return false;
    }
    finished();
    return true;
}

bool WorkItem::cancel() {
    if (!state->cancel(*this)) {
        return false;
    }
    finished();
    return true;
}

void WorkItem::finished() {
    // Runs after the state call has returned, so the mediator may freely act on other items.
    onFinished();
    if (mediator != nullptr) {
        mediator->itemChanged(*this);
    }
}

void WorkItem::onFinished() {
}

bool WorkItem::escalate() {
    return state->escalate(*this);
}

void WorkItem::changeState(WorkItemState* next, EventType event) {
    state.reset(next); // deletes the previous state

    EventDispatcher::getInstance().notifyListeners(
        WorkflowEvent(event, instanceId, itemID, name, outcome));
}

void WorkItem::setAssignee(Participant* participant) {
    assignee = participant;
}

void WorkItem::setOutcome(const std::string& result) {
    outcome = result;
}

void WorkItem::incrementEscalation() {
    ++escalationLevel;
}

bool WorkItem::isFinished() const {
    return state->isFinal();
}

std::string WorkItem::getStateName() const {
    return state->name();
}

std::string WorkItem::getId() const {
    return itemID;
}

std::string WorkItem::getName() const {
    return name;
}

std::string WorkItem::getOutcome() const {
    return outcome;
}

Participant* WorkItem::getAssignee() const {
    return assignee;
}

int WorkItem::getEscalationLevel() const {
    return escalationLevel;
}

std::string WorkItem::getInstanceId() const {
    return instanceId;
}

void WorkItem::setInstanceId(const std::string& id) {
    instanceId = id;
}

SubProcess* WorkItem::getParent() const {
    return parent;
}

void WorkItem::setParent(SubProcess* p) {
    parent = p;
}

WorkflowMediator* WorkItem::getMediator() const {
    return mediator;
}

void WorkItem::setMediator(WorkflowMediator* m) {
    mediator = m;
}

int WorkItem::childCount() const {
    return 0;
}

WorkItem* WorkItem::getChild(int index) const {
    (void)index;
    return nullptr;
}

void WorkItem::setRoutingContext(const InstanceData* data, const ParticipantDirectory* directory) {
    for (int i = 0; i < childCount(); ++i) {
        getChild(i)->setRoutingContext(data, directory);
    }
}
