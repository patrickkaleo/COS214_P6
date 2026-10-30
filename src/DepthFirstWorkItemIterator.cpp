/**
 * @file DepthFirstWorkItemIterator.cpp
 * @brief Implementation of the Iterator hierarchy (13th pattern): DepthFirstWorkItemIterator
 * @details Traverses the Composite tree in depth-first (pre-order) order.
 *          Can also be used to walk structures that are logically chains
 *          of work items (e.g. sequential stages) without exposing children_.
 * @version 1.0
 */

#include "DepthFirstWorkItemIterator.h"
#include "WorkItemIterator.h"
#include "SubProcess.h"
#include "WorkItem.h"
#include "Task.h"

#include <stack>

// DepthFirstWorkItemIterator
DepthFirstWorkItemIterator::DepthFirstWorkItemIterator(SubProcess* root) : root_(root), currentItem_(nullptr) {}

void DepthFirstWorkItemIterator::pushChildren(WorkItem* item) {
    // Only SubProcess has children. Push in reverse so left-to-right pop order
    SubProcess* sub = dynamic_cast<SubProcess*>(item);
    if (!sub) return;

    const std::vector<WorkItem*>& kids = sub->getChildren();
    for (int i = static_cast<int>(kids.size()) - 1; i >= 0; --i) {
        if (kids[static_cast<size_t>(i)]) {
            stack_.push(kids[static_cast<size_t>(i)]);
        }
    }
}

void DepthFirstWorkItemIterator::first() {
    // Clear any previous traversal state
    while (!stack_.empty()) {
        stack_.pop();
    }
    currentItem_ = nullptr;

    if (!root_) return;

    currentItem_ = root_;
    pushChildren(root_);
}

void DepthFirstWorkItemIterator::next() {
    if (stack_.empty()) {
        currentItem_ = nullptr;
        return;
    }
    currentItem_ = stack_.top();
    stack_.pop();
    pushChildren(currentItem_);
}

bool DepthFirstWorkItemIterator::isDone() const {
    return currentItem_ == nullptr;
}

WorkItem* DepthFirstWorkItemIterator::current() const {
    return currentItem_;
}

/* SubProcess::createIterator  (factory for the Iterator pattern)
WorkItemIterator* SubProcess::createIterator() const {
    return new DepthFirstWorkItemIterator(const_cast<SubProcess*>(this));
}*/