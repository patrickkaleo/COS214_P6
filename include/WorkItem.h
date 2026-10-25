#ifndef WORKITEM_H
#define WORKITEM_H

/**
 * @file WorkItem.h
 * @brief Abstract unit of work in a workflow; the Context of the State pattern.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include <memory>
#include <string>
#include "WorkflowEvent.h"

class WorkItemState;
class Participant;
class SubProcess;

/**
 * @class WorkItem
 * @brief Base class for tasks and sub-processes, holding the item's identity and lifecycle.
 *
 * WorkItem is the Context of the GoF State pattern: each lifecycle request is delegated
 * to the current WorkItemState, which decides whether the request is legal and which
 * state follows. WorkItem itself contains no status flags or conditionals over state.
 *
 * Every successful transition is published as a WorkflowEvent through the
 * EventDispatcher singleton (Observer), so audit, notification and monitoring
 * listeners see all state changes.
 *
 * WorkItem is also the Component of the Composite pattern (Task, SubProcess); the
 * composite-specific behaviour lives in those subclasses.
 */
class WorkItem {
protected:
    std::string itemID;        ///< Unique identifier within the workflow definition.
    std::string name;          ///< Human-readable name, e.g. "Faculty Review".
    std::string instanceId;    ///< Owning workflow instance, stamped onto published events.
    std::string outcome;       ///< Completion outcome or rejection reason.
    std::unique_ptr<WorkItemState> state; ///< Current lifecycle state (owned).
    Participant* assignee;     ///< Participant currently responsible (not owned).
    int escalationLevel;       ///< Number of times the item has been escalated.
    SubProcess* parent;        ///< Enclosing sub-process, nullptr for the root (not owned).

public:
    /**
     * @brief Creates a work item in the Created state.
     * @param id Unique item identifier.
     * @param itemName Human-readable name.
     */
    WorkItem(const std::string& id, const std::string& itemName);

    /**
     * @brief Copies identity and lifecycle data; the state object is cloned, not shared.
     */
    WorkItem(const WorkItem& other);

    WorkItem& operator=(const WorkItem&) = delete;

    /// Defined in WorkItem.cpp, where WorkItemState is a complete type for unique_ptr.
    virtual ~WorkItem();

    /// @name Lifecycle requests (delegated to the current WorkItemState)
    /// Each returns true if the transition happened, false if the current state refused it.
    /// @{
    bool makeAvailable();
    bool assign(Participant* participant);
    bool start();
    bool complete(const std::string& outcome);
    bool reject(const std::string& reason);
    bool cancel();
    bool escalate();
    /// @}

    /**
     * @brief Replaces the current state and publishes the matching lifecycle event.
     *
     * Called by concrete states only. The previous state object is deleted, so a state
     * must not touch its own members after calling this.
     *
     * @param next Newly allocated next state (ownership is taken).
     * @param event Event type to publish for this transition.
     */
    void changeState(WorkItemState* next, EventType event);

    /// @name Lifecycle data written by concrete states
    /// @{
    void setAssignee(Participant* participant);
    void setOutcome(const std::string& result);
    void incrementEscalation();
    /// @}

    bool isFinished() const;
    std::string getStateName() const;

    std::string getId() const;
    std::string getName() const;
    std::string getOutcome() const;
    Participant* getAssignee() const;
    int getEscalationLevel() const;

    std::string getInstanceId() const;
    void setInstanceId(const std::string& id);

    SubProcess* getParent() const;
    void setParent(SubProcess* p);

    /**
     * @brief Prototype operation used to copy a definition's tree into a new instance.
     * @return WorkItem* Newly allocated deep copy.
     */
    virtual WorkItem* clone() const = 0;

protected:
    /**
     * @brief Hook run after the item becomes Available.
     *
     * Task uses it to route itself to a participant; SubProcess uses it to make its
     * first children available.
     */
    virtual void onAvailable() = 0;
};

#endif // WORKITEM_H
