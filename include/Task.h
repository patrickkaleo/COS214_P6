/**
 * @file Task.h
 * @brief Leaf of the Composite pattern – atomic work item.
 * @details Holds routing strategy, assignee, service name, checkpoint flag and
 *          escalation level.
 * @version 1.0
 */

#ifndef TASK_H
#define TASK_H

#include "WorkItem.h"
#include <string>

// Forward declarations
class Participant;
class RoutingStrategy;
class InstanceContext;

/**
 * @class Task
 * @brief Concrete leaf WorkItem. Represents a single unit of work that can be
 *        routed, assigned, started and completed.
 */
class Task : public WorkItem {
public:
    /**
     * @brief Constructs a Task with the given identity.
     * @param id   Unique identifier.
     * @param name Human-readable name.
     */
    Task(const std::string& id, const std::string& name);

    /**
     * @brief Destructor. Deletes the owned RoutingStrategy.
     */
    ~Task() override;

    /**
     * @brief Hook: ask the routing strategy for a participant, then assign.
     *        Called by AvailableState after makeAvailable succeeds.
     */
    void onAvailable() override;

    /**
     * @brief Deep-copy this task (including a clone of its routing strategy).
     * @return Newly allocated Task*; caller owns it.
     */
    WorkItem* clone() const override;

    // Task-specific API

    /**
     * @brief Install a routing strategy (takes ownership).
     * @param r Strategy to use when this task becomes available.
     */
    void setRouting(RoutingStrategy* r);

    /**
     * @return The current routing strategy (owned by this Task).
     */
    RoutingStrategy* getRouting() const { return routing_; }

    /**
     * @brief Name of the external service this task may invoke (via ServiceRegistry).
     * @param s Service key string.
     */
    void setServiceName(const std::string& s) { serviceName_ = s; }

    /** @return Configured service name, or empty if none. */
    std::string getServiceName() const { return serviceName_; }

    /**
     * @brief Mark whether a Memento snapshot must be taken before performWork.
     * @param c true = checkpoint.
     */
    void setCheckpoint(bool c) { checkpoint_ = c; }

    /** @return true if this task is a checkpoint. */
    bool isCheckpoint() const { return checkpoint_; }

    /**
     * @brief Current escalation level (used by ApprovalRouting / Chain of Responsibility).
     * @param l Level index (0 = first handler).
     */
    void setEscalationLevel(int l) { escalationLevel_ = l; }

    /** @return Current escalation level. */
    int getEscalationLevel() const { return escalationLevel_; }

    /**
     * @return Participant currently assigned to this task (not owned).
     */
    Participant* getAssignee() const { return assignee_; }

    /**
     * @brief Record the assignee (not owned; lives in ParticipantDirectory).
     * @param p Participant pointer.
     */
    void setAssignee(Participant* p) { assignee_ = p; }

    /**
     * @brief Execute the actual work of the task (may call an ExternalService).
     * @param context Runtime context carrying instance data, services, etc.
     */
    virtual void performWork(InstanceContext& context);

    /**
     * @brief Set the context used by this task's routing strategy.
     * @param data Instance data available during routing (not owned).
     * @param directory Directory used to look up participants (not owned).
     */
	void setRoutingContext(InstanceData* data, ParticipantDirectory* directory);

private:
    Participant* assignee_;          /**< Not owned. */
    RoutingStrategy* routing_;       /**< Owned. */
    std::string serviceName_;        /**< Key into ServiceRegistry. */
    bool checkpoint_;                /**< Take Memento before external call. */
    int escalationLevel_;            /**< For approval / escalation chains. */
	InstanceData* instanceData_;     /**< Not owned. */
	ParticipantDirectory* directory_;/**< Not owned. */
};

#endif // TASK_H
