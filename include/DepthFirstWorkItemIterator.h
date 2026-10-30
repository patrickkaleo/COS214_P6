#ifndef DEPTHFIRSTWORKITEMITERATOR_H
#define DEPTHFIRSTWORKITEMITERATOR_H

/**
 * @file DepthFirstWorkItemIterator.h
 * @brief Pre-order depth-first iterator over a WorkItem tree.
 * @author Angelo Anthony
 * @version 1.0
 */

#include <stack>
#include "WorkItemIterator.h"

/**
 * @class DepthFirstWorkItemIterator
 * @brief ConcreteIterator visiting a work-item tree in pre-order (parent, then children left to right).
 *
 * The order matches how the process reads top to bottom, e.g.
 * root, Intake, Capture, Verify, Review, Decide. It uses only WorkItem::childCount()
 * and WorkItem::getChild(), so it never asks whether an item is a Task or a SubProcess.
 *
 * The iterator does not own the tree, and the tree must not change shape while it
 * is being traversed.
 */
class DepthFirstWorkItemIterator : public WorkItemIterator {
private:
    WorkItem* root;                 ///< Where traversal starts (not owned).
    std::stack<WorkItem*> pending;  ///< Top is the current item; below it, items still to visit.

public:
    /**
     * @brief Creates an iterator positioned on @p root.
     * @param root First item to visit; nullptr gives an empty traversal.
     */
    explicit DepthFirstWorkItemIterator(WorkItem* root);

    void first() override;
    void next() override;
    bool isDone() const override;
    WorkItem* current() const override;
};

#endif // DEPTHFIRSTWORKITEMITERATOR_H
