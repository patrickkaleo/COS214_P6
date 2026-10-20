/**
 * @file WorkflowEventListener.h
 * @brief Abstract Observer base class interface for workflow event subscribers.
 * @details Conforms to the GoF Observer pattern (Observer role) as specified in Section 3.8.
 * @version 1.0
 */

#ifndef WORKFLOW_EVENT_LISTENER_H
#define WORKFLOW_EVENT_LISTENER_H

#include "WorkflowEvent.h"

/**
 * @class WorkflowEventListener
 * @brief Abstract base class defining the Observer interface for receiving workflow notifications.
 * @details Any concrete service (e.g. NotificationService, AuditLogger, MonitoringService)
 *          subscribing to workflow lifecycle events must implement this interface.
 */
class WorkflowEventListener {
public:
    /**
     * @brief Virtual destructor ensuring clean polymorphic destruction of derived listener instances.
     */
    virtual ~WorkflowEventListener() = default;

    /**
     * @brief Callback invoked by subjects (WorkItem) when a lifecycle event transition occurs.
     * @param e Const reference to the WorkflowEvent payload containing event details.
     */
    virtual void onEvent(const WorkflowEvent& e) = 0;
};

#endif // WORKFLOW_EVENT_LISTENER_H
