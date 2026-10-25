#ifndef WORKITEMSTATE_H
#define WORKITEMSTATE_H

/**
 * @file WorkItemState.h
 * @brief Abstract State interface for the work-item lifecycle in the TIVIDY engine.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include <string>

class WorkItem;
class Participant;

/**
 * @class WorkItemState
 * @brief Abstract State defining every lifecycle request a WorkItem can receive.
 *
 * Implements the GoF State (Behavioural) pattern. WorkItem (the Context) delegates
 * makeAvailable(), assign(), start(), complete(), reject(), cancel() and escalate()
 * to its current WorkItemState. A concrete state either performs the transition and
 * moves the item to its next state, or refuses the request.
 *
 * Every operation here refuses by default (returns false), so a concrete state only
 * overrides the transitions that are legal from it. This removes any status flag and
 * if/else chain from WorkItem.
 *
 * Lifecycle:
 * @code
 *   Created --makeAvailable--> Available --assign--> Assigned --start--> InProgress
 *   InProgress --complete--> Completed      InProgress --reject--> Rejected
 *   Available/Assigned/InProgress --escalate--> Escalated --assign--> Assigned
 *   any non-final state --cancel--> Cancelled
 * @endcode
 *
 * @see CreatedState, AvailableState, AssignedState, InProgressState,
 *      EscalatedState, CompletedState, RejectedState, CancelledState
 */
class WorkItemState {
public:
    virtual ~WorkItemState() = default;

    /**
     * @brief Makes the item eligible for assignment.
     * @param item The context work item.
     * @return true if the transition was performed, false if refused.
     */
    virtual bool makeAvailable(WorkItem& item);

    /**
     * @brief Allocates the item to a participant.
     * @param item The context work item.
     * @param participant The participant receiving the work (must not be nullptr).
     * @return true if the transition was performed, false if refused.
     */
    virtual bool assign(WorkItem& item, Participant* participant);

    /**
     * @brief Begins execution of the item by its assignee.
     * @param item The context work item.
     * @return true if the transition was performed, false if refused.
     */
    virtual bool start(WorkItem& item);

    /**
     * @brief Finishes the item successfully with a routing outcome (e.g. "approved").
     * @param item The context work item.
     * @param outcome Outcome used by dependency rules to choose the next work.
     * @return true if the transition was performed, false if refused.
     */
    virtual bool complete(WorkItem& item, const std::string& outcome);

    /**
     * @brief Finishes the item unsuccessfully.
     * @param item The context work item.
     * @param reason Why the work was rejected; recorded as the item's outcome.
     * @return true if the transition was performed, false if refused.
     */
    virtual bool reject(WorkItem& item, const std::string& reason);

    /**
     * @brief Aborts the item.
     * @param item The context work item.
     * @return true if the transition was performed, false if refused.
     */
    virtual bool cancel(WorkItem& item);

    /**
     * @brief Moves the item to a higher authority after a deadline or limit is exceeded.
     * @param item The context work item.
     * @return true if the transition was performed, false if refused.
     */
    virtual bool escalate(WorkItem& item);

    /**
     * @brief Human-readable state name, shown in the TUI and history.
     * @return std::string The state's name, e.g. "InProgress".
     */
    virtual std::string name() const = 0;

    /**
     * @brief Whether the lifecycle has ended in this state.
     * @return true for Completed, Rejected and Cancelled; false otherwise.
     */
    virtual bool isFinal() const;

    /**
     * @brief Creates a copy of this state, used when a WorkItem is copied or restored.
     * @return WorkItemState* Newly allocated state of the same concrete type.
     */
    virtual WorkItemState* clone() const = 0;
};

#endif // WORKITEMSTATE_H
