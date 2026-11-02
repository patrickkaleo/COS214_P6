/**
 * @file NotificationService.h
 * @brief Concrete Observer class responsible for delivering real-time alerts upon workflow events.
 * @details Implements the WorkflowEventListener interface as part of the Observer pattern.
 * @author Enock Nyamweya
 * @version 2.0
 */

#ifndef NOTIFICATION_SERVICE_H
#define NOTIFICATION_SERVICE_H

#include <string>
#include <vector>
#include <iostream>
#include "WorkflowEventListener.h"

/**
 * @class NotificationService
 * @brief Concrete Observer that receives lifecycle events and dispatches user-facing notifications.
 * @details Formats human-readable alert messages based on event transitions (e.g. assignment, completion,
 *          escalation, failure) and maintains an in-memory delivery history.
 */
class NotificationService : public WorkflowEventListener {
private:
    std::string channelName;                  /**< Name of the notification channel (e.g. "Console", "Email", "SMS"). */
    std::vector<std::string> notificationLog; /**< In-memory record of all formatted notifications dispatched. */
    bool echoToConsole;                      /**< Flag indicating whether alerts are printed immediately to std::cout. */

public:
    /**
     * @brief Constructs a new NotificationService instance.
     * @param channel The delivery channel identifier (defaults to "Console").
     * @param echo Whether to immediately print formatted alerts to standard output (defaults to true).
     */
    explicit NotificationService(const std::string& channel = "Console", bool echo = true);

    /**
     * @brief Virtual destructor ensuring clean polymorphic destruction.
     */
    virtual ~NotificationService() override = default;

    /**
     * @brief Observer callback invoked when a workflow lifecycle event occurs.
     * @param e Const reference to the WorkflowEvent payload.
     */
    virtual void onEvent(const WorkflowEvent& e) override;

    /**
     * @brief Formats an event into a user-friendly notification message string.
     * @param e The event to format.
     * @return std::string Formatted alert message.
     */
    std::string formatNotification(const WorkflowEvent& e) const;

    /**
     * @brief Retrieves the complete history of dispatched notification strings.
     * @return const std::vector<std::string>& Reference to the notification log.
     */
    const std::vector<std::string>& getNotificationLog() const;

    /**
     * @brief Returns the count of dispatched notifications.
     * @return size_t Total number of notifications recorded.
     */
    size_t getNotificationCount() const;

    /**
     * @brief Sets whether notifications are automatically printed to standard output.
     * @param echo Boolean toggle for console echo.
     */
    void setEchoToConsole(bool echo);

    /**
     * @brief Clears the internal notification history log.
     */
    void clearLog();
};

#endif // NOTIFICATION_SERVICE_H
