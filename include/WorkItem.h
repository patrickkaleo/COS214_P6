/**
 * @file WorkItem.h
 * @brief Abstract Component of the Composite pattern for the TIVIDY workflow engine.
 * @details Every piece of work (leaf Task or composite SubProcess) is a WorkItem.
 *          Lifecycle operations are forwarded to the current WorkItemState (State pattern).
 *          The class is also the Subject of Observer and a Colleague of the Mediator.
 * @version 1.0
 */

#ifndef WORKITEM_H
#define WORKITEM_H

#include <string>
#include <vector>

// Forward declarations
class Participant;
class WorkItemState;
class SubProcess;
class WorkflowMediator;
class WorkflowEventListener;
class WorkflowEvent;
// class EventType;

#include "WorkflowEvent.h"

/**
 * @class WorkItem
 * @brief Common interface for every piece of work in the engine.
 *
 * Clients and the Mediator only ever talk to a WorkItem*; they never care
 * whether the object is a leaf Task or a composite SubProcess.
 */
class WorkItem
{
public:
    /**
     * @brief Constructs a WorkItem with the given identity.
     * @param id   Unique identifier within a workflow definition / instance.
     * @param name Human-readable name (scenario data, not engine logic).
     */
    WorkItem(const std::string &id, const std::string &name);

    /**
     * @brief Virtual destructor. Deletes the owned WorkItemState.
     */
    virtual ~WorkItem();

    // lifecycle (delegate to state)

    /**
     * @brief Attempt to make this item available for assignment.
     * @return true if the current state accepted the transition.
     */
    virtual bool makeAvailable();

    /**
     * @brief Attempt to assign this item to a participant.
     * @param p Participant to assign (not owned by WorkItem).
     * @return true if the current state accepted the transition.
     */
    virtual bool assign(Participant *p);

    /**
     * @brief Attempt to start work on this item.
     * @return true if the current state accepted the transition.
     */
    virtual bool start();

    /**
     * @brief Attempt to complete this item with the given outcome.
     * @param outcome Result string (e.g. "approved", "paid"). Stored on success.
     * @return true if the current state accepted the transition.
     */
    virtual bool complete(const std::string &outcome);

    /**
     * @brief Attempt to reject this item.
     * @param reason Human-readable rejection reason.
     * @return true if the current state accepted the transition.
     */
    virtual bool reject(const std::string &reason);

    /**
     * @brief Attempt to cancel this item.
     * @return true if the current state accepted the transition.
     */
    virtual bool cancel();

    /**
     * @brief Attempt to escalate this item (e.g. deadline passed).
     * @return true if the current state accepted the transition.
     */
    virtual bool escalate();

    // state management

    /**
     * @brief Replace the current state object (takes ownership).
     * @param s New state; previous state is deleted.
     */
    void setState(WorkItemState *s);

    /**
     * @brief Access the current state object.
     * @return Pointer to the owned WorkItemState (may be nullptr).
     */
    WorkItemState *getState() const { return state_; }

    /**
     * @brief Whether this item is in a terminal state (Completed / Rejected / Cancelled).
     * @return true if no further lifecycle operations are legal.
     */
    bool isFinished() const;

    // identity / data

    /** @return Unique item identifier. */
    std::string getId() const { return id_; }

    /** @return Human-readable name. */
    std::string getName() const { return name_; }

    /** @return Outcome set on successful complete/reject/cancel. */
    std::string getOutcome() const { return outcome_; }

    /**
     * @brief Set the outcome string (normally done by complete/reject/cancel).
     * @param o Outcome value.
     */
    void setOutcome(const std::string &o) { outcome_ = o; }

    // structure

    /**
     * @brief Parent composite, if any.
     * @return Pointer to owning SubProcess, or nullptr for the root.
     */
    SubProcess *getParent() const { return parent_; }

    /**
     * @brief Set the parent composite (not owned).
     * @param p Parent SubProcess.
     */
    void setParent(SubProcess *p) { parent_ = p; }

    // Observer support

    /**
     * @brief Subscribe a listener to lifecycle events on this item.
     * @param l Listener (not owned).
     */
    void attach(WorkflowEventListener *l);

    /**
     * @brief Unsubscribe a listener.
     * @param l Listener previously attached.
     */
    void detach(WorkflowEventListener *l);

    /**
     * @brief Publish an event to all attached listeners.
     * @param event Event payload matching WorkflowEventListener::onEvent.
     */
    void notify(const WorkflowEvent &event);

    // Mediator

    /**
     * @brief Attach the instance mediator (not owned).
     * @param m DependencyMediator or equivalent.
     */
    void setMediator(WorkflowMediator *m) { mediator_ = m; }

    /**
     * @return The mediator currently attached, or nullptr.
     */
    WorkflowMediator *getMediator() const { return mediator_; }

    // Prototype (Builder / instance cloning)

    /**
     * @brief Deep-copy this item (and its sub-tree for composites).
     * @return Newly allocated clone; caller owns it.
     */
    virtual WorkItem *clone() const = 0;

    /**
     * @brief Hook called by AvailableState after a successful makeAvailable transition.
     *        Leaf (Task): performs routing.
     *        Composite (SubProcess): asks the mediator to open entry children.
     */
    virtual void onAvailable() = 0;

    void setAssignee(Participant *participant);

    void changeState(WorkItemState *next, EventType event);

    void incrementEscalation();
    WorkItem(const WorkItem &other);

    std::string getStateName() const;

    Participant *getAssignee() const;

    int getEscalationLevel() const;

    std::string getInstanceId() const;

    void setInstanceId(const std::string &id);

protected:
    std::string instanceId_;
    Participant *assignee_ = nullptr;
    int escalationLevel_ = 0;

    std::string id_;                                 /**< Unique identifier. */
    std::string name_;                               /**< Display name. */
    std::string outcome_;                            /**< Result of completion / rejection. */
    WorkItemState *state_;                           /**< Owned current state. */
    SubProcess *parent_;                             /**< Not owned. */
    std::vector<WorkflowEventListener *> listeners_; /**< Not owned. */
    WorkflowMediator *mediator_;                     /**< Not owned. */
};

#endif // WORKITEM_H
