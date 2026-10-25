#ifndef DEPENDENCYMEDIATOR_H
#define DEPENDENCYMEDIATOR_H

/**
 * @file DependencyMediator.h
 * @brief Concrete Mediator that moves work forward using a definition's dependency rules.
 * @author Angelo Anthony
 * @version 1.0
 */

#include <map>
#include <set>
#include <string>
#include <vector>
#include "WorkflowMediator.h"
#include "DependencyRule.h"

/**
 * @class DependencyMediator
 * @brief ConcreteMediator that decides which work items open next from dependency rules.
 *
 * Each workflow instance owns one DependencyMediator with its own copy of the
 * definition's rules, so instances never affect each other. Order is data: sequence,
 * parallel branches, outcome-based decisions and joins are all expressed as
 * DependencyRule(fromId, toId, outcome), and no process needs its own mediator class.
 *
 * Rules:
 * - When a sub-process starts, each child with no incoming rule opens. Several such
 *   children run in parallel.
 * - A rule is satisfied when its source completes with the rule's outcome. "*"
 *   matches any completion outcome, and "rejected" matches a rejected source.
 * - A rule is dead when its source finishes any other way, i.e. that branch was not
 *   taken.
 * - A target waits until all of its incoming rules are decided (join). It opens if
 *   at least one rule is satisfied. Otherwise it is skipped (cancelled), and the skip
 *   carries on to everything downstream.
 * - A sub-process completes once all of its children have finished.
 * - If an item is rejected and no rule handles "rejected", its parent is rejected
 *   too, which cancels the rest of that branch.
 */
class DependencyMediator : public WorkflowMediator {
private:
    std::vector<DependencyRule> rules;     ///< Copied from the definition.
    std::map<std::string, WorkItem*> items; ///< Registered items by id (not owned).
    std::set<std::string> openedBranches;  ///< Sub-processes whose children may start.
    std::set<std::string> decided;         ///< Items already opened or skipped by this mediator.

    /// Whether a rule's source has finished, and if so whether it took this rule's branch.
    enum class RuleStatus { Pending, Satisfied, Dead };

public:
    /**
     * @brief Creates a mediator for one instance.
     * @param definitionRules The definition's dependency rules (copied).
     */
    explicit DependencyMediator(const std::vector<DependencyRule>& definitionRules = {});

    void registerItem(WorkItem* item) override;

    /**
     * @brief Registers @p root and every item below it, using a depth-first iterator.
     * @param root Root of the instance's work-item tree.
     */
    void registerTree(WorkItem& root);

    void itemChanged(WorkItem& item) override;
    void openChildren(SubProcess& subProcess) override;

    /// @return The registered item with this id, or nullptr.
    WorkItem* findItem(const std::string& id) const;

    /// @return The rules this mediator works from.
    const std::vector<DependencyRule>& getRules() const;

private:
    RuleStatus statusOf(const DependencyRule& rule) const;
    bool hasIncomingRule(const std::string& id) const;

    /// Opens or skips @p target once its branch has started and all incoming rules are decided.
    void evaluate(WorkItem& target);

    /// @return true if some rule leaving @p item was satisfied by how it finished.
    bool hasSatisfiedOutgoingRule(const WorkItem& item) const;

    /// Completes @p subProcess if it is still open and every child has finished.
    void completeIfDone(SubProcess& subProcess);
};

#endif // DEPENDENCYMEDIATOR_H
