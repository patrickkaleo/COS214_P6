/**
 * @file SubProcess.cpp
 * @brief Composite of the Composite pattern
 * @version 1.0
 */

#include "SubProcess.h"
#include "WorkItem.h"
#include "RoutingStrategy.h"
#include "InstanceData.h"
#include "ParticipantDirectory.h"
#include "Participant.h"
#include "ExternalService.h"
#include "ServiceRegistry.h"
// #include "ServiceRequest.h"
// #include "ServiceResult.h"

#include <string>

using namespace std;

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

// SubProcess::createIterator  (factory for the Iterator pattern)
WorkItemIterator* SubProcess::createIterator() const {
    return new DepthFirstWorkItemIterator(const_cast<SubProcess*>(this));
}