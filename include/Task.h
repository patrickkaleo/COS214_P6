/**
 * @file Task.h
 * @brief Leaf of the Composite pattern – atomic work item.
 * @details Holds routing strategy, assignee, service itemName, checkpoint flag and
 *          escalation level.
 * @version 1.0
 */

#ifndef TASK_H
#define TASK_H

#include "WorkItem.h"
#include "InstanceData.h"
#include "ParticipantDirectory.h"

#include <ctime>
#include <string>

// Forward declarations
class RoutingStrategy;
class ServiceRegistry;
class Participant;

/**
 * @class Task
 * @brief Concrete leaf WorkItem. Represents a single unit of work that can be
 *        routed, assigned, started and completed. Atomic work item (Composite leaf).
 */
class Task : public WorkItem {
public:
    /**
     * @brief Constructs a Task with the given identity.
     * @param id   Unique identifier.
     * @param itemName Human-readable name.
     */
    Task(const std::string& id, const std::string& itemName);

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
     * @param strategy Strategy to use when this task becomes available.
     */
    void setRouting(RoutingStrategy* strategy);

    /**
     * @return The current routing strategy (owned by this Task).
     */
    RoutingStrategy* getRouting() const;

    /**
     * @brief Name of the external service this task may invoke (via ServiceRegistry).
     * @param s Service key string.
     */
    void setServiceName(const std::string& s);

    /** @return Configured service itemName, or empty if none. */
    std::string getServiceName() const;

    /**
     * @brief Mark whether a Memento snapshot must be taken before performWork.
     * @param c true = checkpoint.
     */
    void setCheckpoint(bool c);

    /** @return true if this task is a checkpoint. */
    bool isCheckpoint() const;

    /**
     * @brief Set the context used by this task's routing strategy.
     * @param data Instance data available during routing (not owned).
     * @param directory Directory used to look up participants (not owned).
     */
    void setRoutingContext(const InstanceData* data, const ParticipantDirectory* directory) override;
    void setRoutingContext(InstanceData* data, ParticipantDirectory* directory);

    /**
     * @brief Route the task to an assignee based on current data and directory.
     * @return true if successfully assigned or escalated.
     */
    bool route();

    /**
     * @brief Inject service registry for performWork (not owned).
     */
    void setServiceRegistry(ServiceRegistry* registry);

    /**
     * @brief Execute the actual work of the task (may call an ExternalService).
     *
     * The service's outputs are stored in the instance data and the task stays in
     * progress, so its participant can choose the outcome from what came back.
     * A failed or missing service escalates the task.
     *
     * @param context Runtime context carrying instance data, services, etc.
     */
    virtual void performWork(InstanceData& context);

    /**
     * @brief Set how long the task may wait before it escalates.
     * @param hours Hours allowed once the task opens or is escalated; 0 means no deadline.
     */
    void setDeadlineHours(int hours);

    /** @return Hours allowed, or 0 if the task has no deadline. */
    int getDeadlineHours() const;

    /**
     * @brief Escalate the task if its deadline has passed.
     *
     * The routing strategy is then asked again. An approval chain moves to its next level;
     * a strategy that can only offer the same person leaves the task escalated for
     * someone to claim.
     *
     * @param now Time to compare against the deadline.
     * @return true if the task was escalated.
     */
    bool checkDeadline(time_t now);

    /** @return Message from the last service call, or empty if none was made. */
    std::string getServiceMessage() const;

private:
    RoutingStrategy* routing_;             /**< Owned. */
    std::string serviceName_;              /**< Key into ServiceRegistry. */
    std::string serviceMessage_;           /**< Summary of the last service call. */
    int deadlineHours_;                    /**< Hours allowed; 0 = no deadline. */
    time_t dueAt_;                         /**< When the current deadline passes; 0 = not running. */
    bool checkpoint_;                      /**< Take Memento before external call. */
    const InstanceData* instanceData_;     /**< Not owned. */
    const ParticipantDirectory* directory_;/**< Not owned. */
    ServiceRegistry* services_;            /**< Not owned. */
};

#endif // TASK_H
