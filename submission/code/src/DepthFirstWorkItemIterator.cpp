/**
 * @file DepthFirstWorkItemIterator.cpp
 * @brief Pre-order traversal of a WorkItem tree using an explicit stack.
 * @author Angelo Anthony
 * @version 1.0
 */

#include "DepthFirstWorkItemIterator.h"
#include "WorkItem.h"

DepthFirstWorkItemIterator::DepthFirstWorkItemIterator(WorkItem* rootItem)
    : root(rootItem) {
    first();
}

void DepthFirstWorkItemIterator::first() {
    pending = std::stack<WorkItem*>();
    if (root != nullptr) {
        pending.push(root);
    }
}

void DepthFirstWorkItemIterator::next() {
    if (pending.empty()) {
        return;
    }
    WorkItem* visited = pending.top();
    pending.pop();

    // Push children right to left so the leftmost child is visited next.
    for (int i = visited->childCount() - 1; i >= 0; --i) {
        pending.push(visited->getChild(i));
    }
}

bool DepthFirstWorkItemIterator::isDone() const {
    return pending.empty();
}

WorkItem* DepthFirstWorkItemIterator::current() const {
    return pending.empty() ? nullptr : pending.top();
}
