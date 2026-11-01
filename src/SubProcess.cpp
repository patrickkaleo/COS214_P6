/**
 * @file SubProcess.cpp
 * @brief Composite of the Composite pattern
 * @version 1.0
 */

#include "SubProcess.h"
#include "WorkItem.h"
#include "WorkflowMediator.h"
#include "DepthFirstWorkItemIterator.h"
#include "InProgressState.h"
#include "RoutingStrategy.h"
#include "InstanceData.h"
#include "ParticipantDirectory.h"
#include "Participant.h"
#include "ExternalService.h"
#include "ServiceRegistry.h"

#include <string>
#include <algorithm>

using namespace std;

SubProcess::SubProcess(const string& id, const string& name) : WorkItem(id, name) {}

SubProcess::~SubProcess() {
    for (auto* c : children_) {
        delete c;
    }
    children_.clear();
}

void SubProcess::onAvailable() {
    // A container has nobody to assign or start it, so it goes straight to InProgress.
    changeState(new InProgressState(), EventType::STARTED);

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

void SubProcess::onFinished() {
    for (auto* c : children_) {
        if (c && !c->isFinished()) {
            c->cancel();
        }
    }
}

SubProcess* SubProcess::clone() const {
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

bool SubProcess::replaceChild(WorkItem* current, WorkItem* replacement) {
    if (!current || !replacement) {
        return false;
    }
    for (auto& child : children_) {
        if (child == current) {
            replacement->setParent(this);
            child = replacement;
            return true;
        }
    }
    return false;
}

bool SubProcess::remove(const string& id) {
    auto it = find_if(children_.begin(), children_.end(), [&id](WorkItem* w) { return w && w->getId() == id; });
    if (it != children_.end()) {
        delete *it;
        children_.erase(it);
        return true;
    }
    return false;
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

WorkItemIterator* SubProcess::createIterator() const { return new DepthFirstWorkItemIterator(const_cast<SubProcess*>(this)); }

WorkItem* SubProcess::find(const std::string& id) const {
    std::unique_ptr<WorkItemIterator> it(createIterator());
    for (it->first(); !it->isDone(); it->next()) {
        if (it->current() && it->current()->getId() == id) {
            return it->current();
        }
    }
    return nullptr;
}
