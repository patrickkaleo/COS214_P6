#ifndef TASK_H
#define TASK_H

/**
 * @file Task.h
 * @brief Leaf of the Composite pattern: one atomic unit of work.
 * @author Tebogo Mokgwatsane, Angelo Anthony
 * @version 1.1
 */

#include <memory>
#include <string>
#include "WorkItem.h"

class RoutingStrategy;

/**
 * @class Task
 * @brief Concrete leaf WorkItem that is routed to a participant and worked on.
 *
 * When the task becomes Available it asks its RoutingStrategy for a participant and
 * assigns itself. If the strategy finds nobody, the task escalates instead. With
 * ApprovalRouting the strategy is a Chain of Responsibility, and the task's
 * escalation level decides which approver in the chain is asked.
 *
 * Assignee and escalation level are inherited from WorkItem.
 */
class Task : public WorkItem {
private:
    std::unique_ptr<RoutingStrategy> routing;  ///< How a participant is chosen (owned, may be null).
    std::string serviceName;                   ///< ServiceRegistry key of an external system, or empty.
    bool checkpoint;                           ///< Take a snapshot before the external call.
    const InstanceData* data;                  ///< Owning instance's data (not owned).
    const ParticipantDirectory* directory;     ///< Participants to route to (not owned).

public:
    /**
     * @brief Creates a task with no routing; it waits in Available to be claimed manually.
     * @param id Unique item identifier.
     * @param name Human-readable name.
     */
    Task(const std::string& id, const std::string& name);

    /// Deep copy: the routing strategy is cloned; the routing context is not copied.
    Task(const Task& other);

    /// Defined in Task.cpp, where RoutingStrategy is complete for unique_ptr.
    ~Task() override;

    /// @return Newly allocated deep copy (Prototype).
    Task* clone() const override;

    /**
     * @brief Asks the routing strategy for a participant and assigns the task.
     *
     * Called automatically when the task becomes Available. Call it again after
     * escalate() so the next approval level is asked.
     *
     * @return true if the task was assigned or escalated; false if it has no routing,
     *         no routing context, or its state refused the transition.
     */
    bool route();

    /// @param strategy New routing strategy (ownership is taken).
    void setRouting(RoutingStrategy* strategy);
    RoutingStrategy* getRouting() const;

    void setServiceName(const std::string& name);
    std::string getServiceName() const;

    void setCheckpoint(bool isCheckpoint);
    bool isCheckpoint() const;

    void setRoutingContext(const InstanceData* instanceData,
                           const ParticipantDirectory* participants) override;

protected:
    /// Routes the task as soon as it becomes Available.
    void onAvailable() override;
};

#endif // TASK_H
