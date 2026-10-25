/**
 * @file DependencyMediator.cpp
 * @brief Rule evaluation, branch skipping, joins and sub-process completion.
 * @author Angelo Anthony
 * @version 1.0
 */

#include "DependencyMediator.h"
#include "DepthFirstWorkItemIterator.h"
#include "SubProcess.h"

namespace {
const char* const ANY_OUTCOME = "*";             ///< Rule outcome matching any completion.
const char* const REJECTED_OUTCOME = "rejected"; ///< Rule outcome matching a rejected source.
const char* const BRANCH_DONE = "completed";     ///< Outcome given to a finished sub-process.
}

DependencyMediator::DependencyMediator(const std::vector<DependencyRule>& definitionRules)
    : rules(definitionRules) {
}

void DependencyMediator::registerItem(WorkItem* item) {
    if (item == nullptr) {
        return;
    }
    items[item->getId()] = item;
    item->setMediator(this);
}

void DependencyMediator::registerTree(WorkItem& root) {
    DepthFirstWorkItemIterator it(&root);
    for (it.first(); !it.isDone(); it.next()) {
        registerItem(it.current());
    }
}

void DependencyMediator::openChildren(SubProcess& subProcess) {
    openedBranches.insert(subProcess.getId());
    for (int i = 0; i < subProcess.childCount(); ++i) {
        evaluate(*subProcess.getChild(i));
    }
    // Covers an empty branch, or one whose children were all skipped.
    completeIfDone(subProcess);
}

void DependencyMediator::itemChanged(WorkItem& item) {
    if (!item.isFinished()) {
        return;
    }

    SubProcess* parent = item.getParent();
    bool branchOpen = parent != nullptr && !parent->isFinished();

    // Fail the branch before skipping targets; otherwise skipping the last open
    // sibling would let the parent complete instead.
    if (branchOpen && item.getStateName() == "Rejected" && !hasSatisfiedOutgoingRule(item)) {
        // Nothing in the definition recovers from this rejection, so the branch fails.
        // Rejecting the parent cancels its remaining children (SubProcess::onFinished).
        parent->reject(item.getName() + " rejected: " + item.getOutcome());
    }

    for (const DependencyRule& rule : rules) {
        if (rule.fromId == item.getId()) {
            WorkItem* target = findItem(rule.toId);
            if (target != nullptr) {
                evaluate(*target);
            }
        }
    }

    if (parent != nullptr) {
        completeIfDone(*parent);
    }
}

WorkItem* DependencyMediator::findItem(const std::string& id) const {
    std::map<std::string, WorkItem*>::const_iterator found = items.find(id);
    return found == items.end() ? nullptr : found->second;
}

const std::vector<DependencyRule>& DependencyMediator::getRules() const {
    return rules;
}

DependencyMediator::RuleStatus DependencyMediator::statusOf(const DependencyRule& rule) const {
    WorkItem* source = findItem(rule.fromId);
    if (source == nullptr || !source->isFinished()) {
        return RuleStatus::Pending;
    }

    const std::string finalState = source->getStateName();
    if (finalState == "Completed") {
        bool matches = rule.outcome == ANY_OUTCOME || rule.outcome == source->getOutcome();
        return matches ? RuleStatus::Satisfied : RuleStatus::Dead;
    }
    if (finalState == "Rejected") {
        return rule.outcome == REJECTED_OUTCOME ? RuleStatus::Satisfied : RuleStatus::Dead;
    }
    return RuleStatus::Dead; // cancelled or skipped: this branch will never run
}

bool DependencyMediator::hasIncomingRule(const std::string& id) const {
    for (const DependencyRule& rule : rules) {
        if (rule.toId == id) {
            return true;
        }
    }
    return false;
}

void DependencyMediator::evaluate(WorkItem& target) {
    const std::string id = target.getId();
    if (decided.count(id) != 0 || target.isFinished()) {
        return;
    }

    // An item never starts before the branch that contains it.
    SubProcess* parent = target.getParent();
    if (parent != nullptr && openedBranches.count(parent->getId()) == 0) {
        return;
    }

    bool anySatisfied = !hasIncomingRule(id); // entry items need nothing
    for (const DependencyRule& rule : rules) {
        if (rule.toId != id) {
            continue;
        }
        RuleStatus status = statusOf(rule);
        if (status == RuleStatus::Pending) {
            return; // join: wait for every incoming branch to be decided
        }
        if (status == RuleStatus::Satisfied) {
            anySatisfied = true;
        }
    }

    // Mark before acting: opening or skipping can re-enter this mediator.
    decided.insert(id);
    if (anySatisfied) {
        target.makeAvailable();
    } else {
        target.cancel();
    }
}

bool DependencyMediator::hasSatisfiedOutgoingRule(const WorkItem& item) const {
    for (const DependencyRule& rule : rules) {
        if (rule.fromId == item.getId() && statusOf(rule) == RuleStatus::Satisfied) {
            return true;
        }
    }
    return false;
}

void DependencyMediator::completeIfDone(SubProcess& subProcess) {
    if (!subProcess.isFinished() && subProcess.allChildrenFinished()) {
        subProcess.complete(BRANCH_DONE);
    }
}
