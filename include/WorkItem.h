/**
 * @file WorkItem.h
 * @brief Common interface/base for every piece of work in the engine - Task (leaf) and SubProcess (composite).
 * @version 1.0
 */

#ifndef WORKITEM_H
#define WORKITEM_H

#include <string>
#include <vector>
#include <algorithm>

#include "WorkflowEvent.h"   // EventType

// Forward declarations
class Participant;
class WorkItemState;
class SubProcess;
class WorkflowMediator;
class WorkflowEventListener;
class InstanceData;
class ParticipantDirectory;

/**
 * @class WorkItem
 * @brief Common interface/base for every piece of work in the engine - Task (leaf) and SubProcess (composite).
 *
 * Clients and the Mediator only ever talk to a WorkItem*; they never care
 * whether the object is a leaf Task or a composite SubProcess.
 */
class WorkItem {
public:
    /**
     * @brief Constructs a WorkItem with the given identity.
     * @param id   Unique identifier within a workflow definition / instance.
     * @param name Human-readable name (scenario data, not engine logic).
     */
    WorkItem(const std::string& id, const std::string& name);

    /**
     * @brief Copy constructs a WorkItem with the given identity.
     * @param other workItem to be copied.
     */
    WorkItem(const WorkItem& other);

    /**
     * @brief Virtual destructor. Cleans up owned state.
     */
    virtual ~WorkItem();

    // Lifecycle operations (delegated to state_)

    /**
     * @brief Attempt to make this item available for assignment.
     * @return true if the current state permits the transition.
     */
    virtual bool makeAvailable();

    /**
     * @brief Attempt to assign this item to a participant.
     * @param p Participant to assign (not owned).
     * @return true if assignment succeeded.
     */
    virtual bool assign(Participant* p);

    /**
     * @brief Attempt to start work on this item.
     * @return true if started.
     */
    virtual bool start();

    /**
     * @brief Attempt to complete this item with an outcome.
     * @param outcome Result string (e.g. "approved", "rejected", "verified").
     * @return true if completion succeeded.
     */
    virtual bool complete(const std::string& outcome);

    /**
     * @brief Attempt to reject this item with a reason.
     * @param reason Explanation for rejection.
     * @return true if rejection succeeded.
     */
    virtual bool reject(const std::string& reason);

    /**
     * @brief Attempt to cancel this item.
     * @return true if cancelled.
     */
    virtual bool cancel();

    /**
     * @brief Attempt to escalate this item (e.g. deadline passed).
     * @return true if escalation succeeded.
     */
    virtual bool escalate();

    // State inspection / mutation

    /**
     * @brief Transition to a new state and publish the associated WorkflowEvent.
     * @param next  Newly allocated state object (WorkItem takes ownership).
     * @param event Event type corresponding to the transition.
     */
    void changeState(WorkItemState* next, EventType event);

    /**
     * @brief Replace the state object directly without firing a lifecycle event.
     *        Used by Memento restore and initial state wiring.
     * @param s New state (takes ownership; deletes old state).
     */
    void setState(WorkItemState* s);

    /**
     * @brief Access the current state object.
     * @return Pointer to the owned WorkItemState (may be nullptr).
     */
    WorkItemState* getState() const { return state_; }

    /** @brief Sets the participant assigned to this work item.
     *  @param participant Participant to assign; ownership is not transferred.
     */
    void setAssignee(Participant* participant);

    /**
     * @brief Set the outcome string (normally done by complete/reject/cancel).
     * @param o Outcome value.
     */
    void setOutcome(const std::string& o) { outcome_ = o; }

    /** @brief Increases the escalation level by one.
     */
    void incrementEscalation();

    /**
     * @brief Whether this item is in a terminal state (Completed / Rejected / Cancelled).
     * @return true if no further lifecycle operations are legal.
     */
    bool isFinished() const;

    /**
     * @brief Human-readable name of the current state.
     * @return State name string (e.g. "Created", "Available", "Assigned", ...).
     */
    std::string getStateName() const;

    // Getters and Setters
    std::string getId() const { return id_; }
    std::string getName() const { return name_; }
    std::string getOutcome() const { return outcome_; }

    /** @brief Gets the participant assigned to this work item.
     *  @return The assigned participant, or nullptr if none is assigned.
     */
    Participant* getAssignee() const { return assignee; }

    /** @brief Gets the current escalation level.
     *  @return The escalation level.
     */
    int getEscalationLevel() const { return escalationLevel; }

    /** @brief Sets the escalation level directly.
     *  @param level The new escalation level.
     */
    void setEscalationLevel(int level) { escalationLevel = level; }

    /** @brief Gets the unique identifier for this work-item instance.
     *  @return The instance identifier.
     */
    std::string getInstanceId() const { return instanceId; }

    /** @brief Sets the unique identifier for this work-item instance.
     *  @param id The instance identifier to store.
     */
    void setInstanceId(const std::string& id) { instanceId = id; }

    // structure

    /**
     * @brief Parent composite, if any.
     * @return Pointer to owning SubProcess, or nullptr for the root.
     */
    SubProcess* getParent() const { return parent_; }

    /**
     * @brief Set the parent composite (not owned).
     * @param p Parent SubProcess.
     */
    void setParent(SubProcess* p) { parent_ = p; }

    // Observer support

    /**
     * @brief Subscribe a listener to lifecycle events on this item.
     * @param l Listener (not owned).
     */
    void attach(WorkflowEventListener* l);

    /**
     * @brief Unsubscribe a listener.
     * @param l Listener previously attached.
     */
    void detach(WorkflowEventListener* l);

    /**
     * @brief Publish an event to all attached listeners.
     * @param event Event payload matching WorkflowEventListener::onEvent.
     */
    void notify(const WorkflowEvent& event);

    // Mediator

    /**
     * @brief Attach the instance mediator (not owned).
     * @param m DependencyMediator or equivalent.
     */
    void setMediator(WorkflowMediator* m) { mediator_ = m; }

    /**
     * @return The mediator currently attached, or nullptr.
     */
    WorkflowMediator* getMediator() const { return mediator_; }

    /// @name Uniform child access (Composite)
    /// A leaf has no children; SubProcess overrides both.
    /// @{
    virtual int childCount() const;
    virtual WorkItem* getChild(int index) const;
    /// @}

    /**
     * @brief Recursively propagates routing context down the workflow tree.
     * @param data Instance runtime data.
     * @param directory Participant directory.
     */
    virtual void setRoutingContext(const InstanceData* data, const ParticipantDirectory* directory);

    // Prototype (Builder / instance cloning)

    /**
     * @brief Deep-copy this item (and its sub-tree for composites).
     * @return Newly allocated clone; caller owns it.
     */
    virtual WorkItem* clone() const = 0;

    /**
     * @brief Hook called by AvailableState after a successful makeAvailable transition.
     *        Leaf (Task): performs routing.
     *        Composite (SubProcess): asks the mediator to open entry children.
     */
    virtual void onAvailable() = 0;

    /**
     * @brief Hook run after the item reaches a final state, before mediator is told.
     */
    virtual void onFinished();

protected:
    void finished();
    std::string id_;                                       /**< Unique identifier. */
    std::string name_;                                     /**< Display name. */
    std::string instanceId;
    std::string outcome_;                                  /**< Result of completion / rejection. */
    WorkItemState* state_;                                 /**< Owned current state. */
    Participant* assignee;   
    int escalationLevel;
    SubProcess* parent_;                                   /**< Not owned. */
    std::vector<WorkflowEventListener*> listeners_;        /**< Not owned. */
    WorkflowMediator* mediator_;                           /**< Not owned. */
};

#endif // WORKITEM_H
