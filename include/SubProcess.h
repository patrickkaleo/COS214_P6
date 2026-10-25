#ifndef SUBPROCESS_H
#define SUBPROCESS_H

/**
 * @file SubProcess.h
 * @brief Composite of the Composite pattern: a branch of work that owns child WorkItems.
 * @author Tebogo Mokgwatsane, Angelo Anthony
 * @version 1.1
 */

#include <memory>
#include <string>
#include <vector>
#include "WorkItem.h"
#include "WorkItemIterator.h"

/**
 * @class SubProcess
 * @brief Composite WorkItem containing Tasks and nested SubProcesses.
 *
 * A sub-process has no assignee of its own. Making it available moves it straight
 * to InProgress and asks the mediator to open its entry children. The mediator
 * completes it once every child has finished. If it is rejected or cancelled, any
 * child still open is cancelled with it.
 *
 * Children are owned and destroyed with their parent; clone() copies the whole branch.
 */
class SubProcess : public WorkItem {
private:
    std::vector<std::unique_ptr<WorkItem>> children; ///< Owned children, in definition order.

public:
    SubProcess(const std::string& id, const std::string& name);

    /// Deep copy: every child is cloned and re-parented to the copy.
    SubProcess(const SubProcess& other);

    ~SubProcess() override;

    /// @return Newly allocated deep copy of this branch (Prototype).
    SubProcess* clone() const override;

    /**
     * @brief Appends a child and takes ownership of it.
     * @param child Task or nested SubProcess; nullptr is ignored.
     */
    void add(WorkItem* child);

    /**
     * @brief Removes and deletes the direct child with the given id.
     * @param id Identifier of the child.
     * @return true if a child was removed.
     */
    bool remove(const std::string& id);

    int childCount() const override;

    /// @return The child at @p index, or nullptr if out of range.
    WorkItem* getChild(int index) const override;

    /// @return true if every direct child is in a final state (true when there are none).
    bool allChildrenFinished() const;

    /**
     * @brief Finds an item anywhere in this branch, including the sub-process itself.
     * @param id Identifier to look for.
     * @return The item, or nullptr if it is not in this branch.
     */
    WorkItem* find(const std::string& id);

    /**
     * @brief Creates a pre-order depth-first iterator over this branch.
     * @return Iterator whose first element is this sub-process.
     */
    std::unique_ptr<WorkItemIterator> createIterator();

protected:
    /// Moves to InProgress and asks the mediator to open the entry children.
    void onAvailable() override;

    /// Cancels every child that has not finished yet.
    void onFinished() override;
};

#endif // SUBPROCESS_H
