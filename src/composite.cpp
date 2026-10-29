/**
 * @file composite.cpp
 * @brief Implementation of the Composite hierarchy: Task & SubProcess.
 * @version 1.0
 */

#include "WorkItem.h"
#include "Task.h"
#include "SubProcess.h"

#include "WorkItemState.h"
#include "WorkflowMediator.h"
#include "WorkflowEventListener.h"
#include "WorkflowEvent.h"
#include "Participant.h"
#include "RoutingStrategy.h"
#include "InstanceData.h"
#include "ParticipantDirectory.h"
//#include "InstanceContext.h"  

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// WorkItem
WorkItem::WorkItem(const string& id, const string& name)
    : id_(id),
      name_(name),
      outcome_(""),
      state_(nullptr), //engine / Builder will set a real CreatedState
      parent_(nullptr),
      mediator_(nullptr)
{}

WorkItem::~WorkItem() {
    delete state_;
    // listeners_ and mediator_ are not owned
}

bool WorkItem::makeAvailable() {
    if (!state_) return false;
    bool ok = state_->makeAvailable(*this);
    if (ok) {
        notify(WorkflowEvent(EventType::AVAILABLE, "", id_, name_, ""));
        onAvailable(); 
    }
    return ok;
}

bool WorkItem::assign(Participant* p) {
    if (!state_) return false;
    bool ok = state_->assign(*this, p);
    if (ok) {
        notify(WorkflowEvent(EventType::ASSIGNED, "", id_, name_, ""));
    }
    return ok;
}

bool WorkItem::start() {
    if (!state_) return false;
    bool ok = state_->start(*this);
    if (ok) {
        notify(WorkflowEvent(EventType::STARTED, "", id_, name_, ""));
    }
    return ok;
}

bool WorkItem::complete(const string& outcome) {
    if (!state_) return false;
    bool ok = state_->complete(*this, outcome);
    if (ok) {
        outcome_ = outcome;
        notify(WorkflowEvent(EventType::COMPLETED, "", id_, name_, outcome_));
        if (mediator_) {
            mediator_->itemChanged(*this);
        }
    }
    return ok;
}

bool WorkItem::reject(const string& reason) {
    if (!state_) return false;
    bool ok = state_->reject(*this, reason);
    if (ok) {
        outcome_ = "rejected";
        notify(WorkflowEvent(EventType::REJECTED, "", id_, name_, reason));
        if (mediator_) {
            mediator_->itemChanged(*this);
        }
    }
    return ok;
}

bool WorkItem::cancel() {
    if (!state_) return false;
    bool ok = state_->cancel(*this);
    if (ok) {
        outcome_ = "cancelled";
        notify(WorkflowEvent(EventType::CANCELLED, "", id_, name_, outcome_));
        if (mediator_) {
            mediator_->itemChanged(*this);
        }
    }
    return ok;
}

bool WorkItem::escalate() {
    if (!state_) return false;
    bool ok = state_->escalate(*this);
    if (ok) {
        notify(WorkflowEvent(EventType::ESCALATED, "", id_, name_, ""));
    }
    return ok;
}

void WorkItem::setState(WorkItemState* s) {
    if (state_ == s) return;
    delete state_;
    state_ = s;
}

bool WorkItem::isFinished() const {
    if (!state_) return false;
    const string n = state_->name();
    return n == "Completed" || n == "Rejected" || n == "Cancelled";
}

void WorkItem::attach(WorkflowEventListener* l) {
    if (l) {
        listeners_.push_back(l);
    }
}

void WorkItem::detach(WorkflowEventListener* l) {
    listeners_.erase( remove(listeners_.begin(), listeners_.end(), l), listeners_.end());
}

void WorkItem::notify(const WorkflowEvent& event) {
    for (auto* l : listeners_) {
        if (l) {
            l->onEvent(event);
        }
    }
}

// Task (Leaf)
Task::Task(const string& id, const string& name)
    : WorkItem(id, name),
      assignee_(nullptr),
      routing_(nullptr),
      serviceName_(""),
      checkpoint_(false),
      escalationLevel_(0),
      instanceData_(nullptr),
      directory_(nullptr)
{}

Task::~Task() {
    delete routing_;
}

void Task::setRoutingContext(InstanceData* data, ParticipantDirectory* directory) {
    instanceData_ = data;
    directory_ = directory;
}

void Task::onAvailable() {
    if (!routing_) return;
    if (!instanceData_ || !directory_) return;

    Participant* p = routing_->select(*this, *instanceData_, *directory_);
    if (p) {
        if (assign(p)) {
            setAssignee(p);
        }
    } else {
        escalate();
    }
}

WorkItem* Task::clone() const {
    Task* copy = new Task(id_, name_);
    copy->serviceName_ = serviceName_;
    copy->checkpoint_ = checkpoint_;
    copy->escalationLevel_ = escalationLevel_;
    if (routing_) {
        copy->routing_ = routing_->clone();
    }
    // state/parent/mediator/listeners are wired by the instance after clone
    return copy;
}

void Task::setRouting(RoutingStrategy* r) {
    delete routing_;
    routing_ = r;
}

void Task::performWork(InstanceContext& /*context*/) {
    cout << "[Task::performWork] " << name_ << "\n";
    // Implementation: look up serviceName_ in ServiceRegistry, call the adapter, complete(...) or restore a snapshot on failure.
}

// SubProcess (Composite)
SubProcess::SubProcess(const string& id, const string& name) : WorkItem(id, name) {}

SubProcess::~SubProcess() {
    for (auto* c : children_) {
        delete c;
    }
    children_.clear();
}

void SubProcess::onAvailable() {
    // Ask the mediator to open the entry children  
    if (mediator_) {
        mediator_->openChildren(*this);
    } else {
        // Fallback when no mediator is attached yet
        for (auto* c : children_) {
            if (c) {
                c->makeAvailable();
            }
        }
    }
}

WorkItem* SubProcess::clone() const {
    SubProcess* copy = new SubProcess(id_, name_);
    for (const auto* c : children_) {
        if (c) {
            copy->add(c->clone());   // add() sets parent
        }
    }
    return copy;
}

void SubProcess::add(WorkItem* child) {
    if (!child) return;
    child->setParent(this);
    children_.push_back(child);
}

void SubProcess::remove(const string& id) {
    auto it = find_if(children_.begin(), children_.end(), [&id](WorkItem* w) { return w && w->getId() == id; });
    if (it != children_.end()) {
        delete *it;
        children_.erase(it);
    }
}

WorkItem* SubProcess::getChild(int index) const {
    if (index < 0 || index >= static_cast<int>(children_.size())) {
        return nullptr;
    }
    return children_[static_cast<size_t>(index)];
}

bool SubProcess::allChildrenFinished() const {
    if (children_.empty()) return true;
    for (const auto* c : children_) {
        if (c && !c->isFinished()) {
            return false;
        }
    }
    return true;
}
