/**
 * @file SubProcess.h
 * @brief Composite of the Composite pattern – owns child WorkItems.
 * @details A SubProcess can contain Tasks and nested SubProcesses. Operations
 *          such as makeAvailable / cancel / clone work uniformly on the whole
 *          branch.
 * @version 1.0
 */

#ifndef SUBPROCESS_H
#define SUBPROCESS_H

#include <memory>
#include <string>
#include <vector>

#include "WorkItem.h"

class WorkItemIterator;

/**
 * @class SubProcess
 * @brief Composite WorkItem that owns a collection of children.
 *
 * Children are destroyed with the parent. clone() deep-copies the entire sub-tree.
 * onAvailable() asks the mediator to open entry children (those with no incoming rule).
 * createIterator() supplies a depth-first Iterator over this branch.
 */
class SubProcess : public WorkItem {
public:
    /**
     * @brief Constructs a SubProcess with the given identity.
     * @param id   Unique identifier.
     * @param name Human-readable name.
     */
    SubProcess(const std::string& id, const std::string& name);

    /**
     * @brief Destructor. Deletes every owned child WorkItem.
     */
    ~SubProcess() override;

    /**
     * @brief Hook: ask the mediator to open entry children.
     *        Called by AvailableState after makeAvailable succeeds.
     */
    void onAvailable() override;

    /**
     * @brief Hook called when SubProcess finishes; cancels any children still open.
     */
    void onFinished() override;

    /**
     * @brief Deep-copy this sub-process and all of its children.
     * @return Newly allocated SubProcess*; caller owns it.
     */
    SubProcess* clone() const override;

    // Composite operations

    /**
     * @brief Add a child (takes ownership). Sets the child's parent to this.
     * @param child WorkItem to own (Task or nested SubProcess).
     */
    void add(WorkItem* child);

    /**
     * @brief Remove and delete the child with the given id.
     * @param id Identifier of the child to remove.
     * @return true if a child was removed, false otherwise.
     */
    bool remove(const std::string& id);

    /**
     * @brief Access a child by zero-based index.
     * @param index Child index.
     * @return Pointer to the child, or nullptr if out of range.
     */
    WorkItem* getChild(int index) const override;

    /**
     * @return Number of direct children.
     */
    int childCount() const override { return static_cast<int>(children_.size()); }

    /**
     * @brief Whether every direct child is in a terminal state.
     * @return true if there are no children or all are finished.
     */
    bool allChildrenFinished() const;

    /**
     * @return Const reference to the internal children vector.
     */
    const std::vector<WorkItem*>& getChildren() const { return children_; }

    /**
     * @brief Finds an item anywhere in this branch, including this sub-process.
     * @param id Identifier to look for.
     * @return The item, or nullptr if not in this branch.
     */
    WorkItem* find(const std::string& id) const;

    // Iterator

    /**
     * @brief Factory for a depth-first iterator over this sub-tree.
     * @return Unique pointer to newly allocated WorkItemIterator.
     * @note Concrete type is DepthFirstWorkItemIterator.
     */
    WorkItemIterator* createIterator() const;

private:
    std::vector<WorkItem*> children_;   /**< Owned children. */
};

#endif // SUBPROCESS_H
