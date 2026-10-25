/**
 * @file SubProcess.cpp
 * @brief Child management, traversal and lifecycle hooks of the composite work item.
 * @author Tebogo Mokgwatsane, Angelo Anthony
 * @version 1.1
 */

#include "SubProcess.h"
#include "DepthFirstWorkItemIterator.h"
#include "InProgressState.h"
#include "WorkflowMediator.h"

SubProcess::SubProcess(const std::string& id, const std::string& name)
    : WorkItem(id, name) {
}

SubProcess::SubProcess(const SubProcess& other)
    : WorkItem(other) {
    for (const std::unique_ptr<WorkItem>& child : other.children) {
        add(child->clone());
    }
}

SubProcess::~SubProcess() = default;

SubProcess* SubProcess::clone() const {
    return new SubProcess(*this);
}

void SubProcess::add(WorkItem* child) {
    if (child == nullptr) {
        return;
    }
    child->setParent(this);
    children.emplace_back(child);
}

bool SubProcess::remove(const std::string& id) {
    for (auto it = children.begin(); it != children.end(); ++it) {
        if ((*it)->getId() == id) {
            children.erase(it);
            return true;
        }
    }
    return false;
}

int SubProcess::childCount() const {
    return static_cast<int>(children.size());
}

WorkItem* SubProcess::getChild(int index) const {
    if (index < 0 || index >= childCount()) {
        return nullptr;
    }
    return children[index].get();
}

bool SubProcess::allChildrenFinished() const {
    for (const std::unique_ptr<WorkItem>& child : children) {
        if (!child->isFinished()) {
            return false;
        }
    }
    return true;
}

WorkItem* SubProcess::find(const std::string& id) {
    std::unique_ptr<WorkItemIterator> it = createIterator();
    for (it->first(); !it->isDone(); it->next()) {
        if (it->current()->getId() == id) {
            return it->current();
        }
    }
    return nullptr;
}

std::unique_ptr<WorkItemIterator> SubProcess::createIterator() {
    return std::unique_ptr<WorkItemIterator>(new DepthFirstWorkItemIterator(this));
}

void SubProcess::onAvailable() {
    // A container has nobody to assign or start it, so it goes straight to InProgress.
    changeState(new InProgressState(), EventType::STARTED);
    if (mediator != nullptr) {
        mediator->openChildren(*this);
    }
}

void SubProcess::onFinished() {
    for (const std::unique_ptr<WorkItem>& child : children) {
        if (!child->isFinished()) {
            child->cancel();
        }
    }
}
