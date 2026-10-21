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

WorkItem::WorkItem(const std::string& id, const std::string& itemName)
    : itemID(id),
      name(itemName),
      instanceId(""),
      outcome(""),
      state(new CreatedState()),
      assignee(nullptr),
      escalationLevel(0),
      parent(nullptr) {
}

WorkItem::WorkItem(const WorkItem& other)
    : itemID(other.itemID),
      name(other.name),
      instanceId(other.instanceId),
      outcome(other.outcome),
      state(other.state->clone()),
      assignee(other.assignee),
      escalationLevel(other.escalationLevel),
      parent(nullptr) { // the copying composite re-links its children
}

WorkItem::~WorkItem() {
    delete state;
}

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
    return state->complete(*this, result);
}

bool WorkItem::reject(const std::string& reason) {
    return state->reject(*this, reason);
}

bool WorkItem::cancel() {
    return state->cancel(*this);
}

bool WorkItem::escalate() {
    return state->escalate(*this);
}

void WorkItem::changeState(WorkItemState* next, EventType event) {
    WorkItemState* previous = state;
    state = next;
    delete previous;

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
