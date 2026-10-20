/**
 * @file WorkflowEvent.h
 * @brief Defines the event types and data payload structure for workflow lifecycle notifications.
 * @details Implements the Observer pattern event transport mechanism as specified in Section 3.8.
 * @version 1.0
 */

#ifndef WORKFLOW_EVENT_H
#define WORKFLOW_EVENT_H

#include <string>
#include <ctime>

/**
 * @enum EventType
 * @brief Enumeration of all discrete lifecycle transition events in the TIVIDY workflow engine.
 */
enum class EventType {
    AVAILABLE,          /**< Work item has become eligible for execution/assignment. */
    ASSIGNED,           /**< Work item has been allocated to a participant. */
    STARTED,            /**< Execution of the work item has commenced. */
    COMPLETED,          /**< Work item has successfully executed and finished. */
    REJECTED,           /**< Work item review or validation was unsuccessful. */
    CANCELLED,          /**< Work item has been aborted or superseded. */
    ESCALATED,          /**< Work item exceeded deadline or authority threshold and was escalated. */
    FAILED,             /**< External service call or checkpoint execution encountered an error. */
    INSTANCE_COMPLETED  /**< Entire workflow instance root process has finished. */
};

/**
 * @brief Utility function to convert an EventType enum value to its string representation.
 * @param type The EventType value to convert.
 * @return std::string The uppercase string representation of the event type.
 */
inline std::string eventTypeToString(EventType type) {
    switch (type) {
        case EventType::AVAILABLE:          return "AVAILABLE";
        case EventType::ASSIGNED:           return "ASSIGNED";
        case EventType::STARTED:            return "STARTED";
        case EventType::COMPLETED:          return "COMPLETED";
        case EventType::REJECTED:           return "REJECTED";
        case EventType::CANCELLED:          return "CANCELLED";
        case EventType::ESCALATED:          return "ESCALATED";
        case EventType::FAILED:             return "FAILED";
        case EventType::INSTANCE_COMPLETED: return "INSTANCE_COMPLETED";
        default:                            return "UNKNOWN";
    }
}

/**
 * @struct WorkflowEvent
 * @brief Immutable value object carrying contextual data about a workflow lifecycle event.
 * @details Dispatched by WorkItem subjects and delivered by const reference to registered WorkflowEventListeners.
 */
struct WorkflowEvent {
    EventType type;         /**< The category/type of lifecycle transition. */
    std::string instanceId; /**< Identifier of the workflow instance triggering the event. */
    std::string itemId;     /**< Unique identifier of the work item originating the event. */
    std::string itemName;   /**< Human-readable descriptive name of the work item. */
    std::string outcome;    /**< Result or reason associated with completion/rejection (e.g. "approved"). */
    time_t timestamp;       /**< Unix epoch timestamp when the event occurred. */

    /**
     * @brief Constructs a new WorkflowEvent.
     * @param t The EventType of the transition.
     * @param instId The ID of the workflow instance.
     * @param itId The ID of the work item.
     * @param itName The descriptive name of the work item.
     * @param out Optional outcome or rejection reason string (defaults to empty).
     * @param time Unix timestamp of event occurrence (defaults to current system time).
     */
    WorkflowEvent(EventType t,
                  const std::string& instId,
                  const std::string& itId,
                  const std::string& itName,
                  const std::string& out = "",
                  time_t time = std::time(nullptr))
        : type(t), instanceId(instId), itemId(itId), itemName(itName), outcome(out), timestamp(time) {}
};

#endif // WORKFLOW_EVENT_H
