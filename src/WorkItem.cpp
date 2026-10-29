/**
 * @file WorkItem.cpp
 * @brief WorkItem implementation aligned with WorkItem.h (State + Composite + Observer hooks).
 * @details Lifecycle delegates to WorkItemState. changeState() updates state and publishes
 *          via EventDispatcher. makeAvailable() calls onAvailable() for Composite (Task routing /
 *          SubProcess open children). Optional per-item listeners via attach/detach/notify.
 * @version 1.0
 */

#include "WorkItem.h"
#include "WorkItemState.h"
#include "CreatedState.h"
#include "EventDispatcher.h"
#include "WorkflowEvent.h"
#include "Participant.h"
#include "SubProcess.h"

#include <algorithm>
#include <string>

using namespace std;

// Construction / destruction
WorkItem::WorkItem(const string& id, const string& name)
    : id_(id),
      name_(name),
      instanceId(""),
      outcome_(""),
      state_(new CreatedState()),
      assignee(nullptr),
      escalationLevel(0),
      parent_(nullptr),
      mediator_(nullptr)
{}

WorkItem::WorkItem(const WorkItem& other)
    : id_(other.id_),
      name_(other.name_),
      instanceId(other.instanceId),
      outcome_(other.outcome_),
      state_(other.state_ ? other.state_->clone() : nullptr),
      assignee(other.assignee),
      escalationLevel(other.escalationLevel),
      parent_(nullptr),
      mediator_(nullptr)
      // listeners_ not copied; instance re-attaches as needed
{}

WorkItem::~WorkItem() {
    delete state_;
}

// Lifecycle
bool WorkItem::makeAvailable() {
    if (!state_) return false;
    if (!state_->makeAvailable(*this)) {
        return false;
    }
    onAvailable();
    return true;
}

bool WorkItem::assign(Participant* p) {
    if (!state_) return false;
    return state_->assign(*this, p);
}

bool WorkItem::start() {
    if (!state_) return false;
    return state_->start(*this);
}

bool WorkItem::complete(const string& outcome) {
    if (!state_) return false;
    return state_->complete(*this, outcome);
}

bool WorkItem::reject(const string& reason) {
    if (!state_) return false;
    return state_->reject(*this, reason);
}

bool WorkItem::cancel() {
    if (!state_) return false;
    return state_->cancel(*this);
}

bool WorkItem::escalate() {
    if (!state_) return false;
    return state_->escalate(*this);
}

// State transition + EventDispatcher (per-item listeners)
void WorkItem::changeState(WorkItemState* next, EventType event) {
    if (state_ == next) return;
    delete state_;
    state_ = next;

    WorkflowEvent ev(event, instanceId, id_, name_, outcome_);
    EventDispatcher::getInstance().notifyListeners(ev);
    notify(ev);
}

void WorkItem::setState(WorkItemState* s) {
    if (state_ == s) return;
    delete state_;
    state_ = s;
}

// Helpers
void WorkItem::setAssignee(Participant* participant) {
    assignee = participant;
}

void WorkItem::incrementEscalation() {
    ++escalationLevel;
}

bool WorkItem::isFinished() const {
    if (!state_) return false;
    return state_->isFinal();
}

string WorkItem::getStateName() const {
    if (!state_) return "";
    return state_->name();
}

Participant* WorkItem::getAssignee() const {
    return assignee;
}

int WorkItem::getEscalationLevel() const {
    return escalationLevel;
}

string WorkItem::getInstanceId() const {
    return instanceId;
}

void WorkItem::setInstanceId(const string& id) {
    instanceId = id;
}

// Per-item Observer
void WorkItem::attach(WorkflowEventListener* l) {
    if (!l) return;
    if (find(listeners_.begin(), listeners_.end(), l) != listeners_.end()) {
        return;
    }
    listeners_.push_back(l);
}

void WorkItem::detach(WorkflowEventListener* l) {
    listeners_.erase(
        remove(listeners_.begin(), listeners_.end(), l),
        listeners_.end());
}

void WorkItem::notify(const WorkflowEvent& event) {
    for (auto* listener : listeners_) {
        if (listener) {
            listener->onEvent(event);
        }
    }
}