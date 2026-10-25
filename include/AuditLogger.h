/**
 * @file AuditLogger.h
 * @brief Concrete Observer class responsible for recording an immutable audit trail of workflow events.
 * @details Implements the WorkflowEventListener interface as part of the Observer pattern (Section 3.8).
 * @version 1.0
 */

#ifndef AUDIT_LOGGER_H
#define AUDIT_LOGGER_H

#include <string>
#include <vector>
#include <iostream>
#include "WorkflowEventListener.h"

/**
 * @class AuditLogger
 * @brief Concrete Observer that maintains a persistent, chronological audit log of all workflow transitions.
 * @details Ensures compliance, non-repudiation, and diagnostic traceability by timestamping and recording
 *          every state change, task execution, decision outcome, and workflow termination.
 */
class AuditLogger : public WorkflowEventListener {
private:
    std::vector<std::string> auditTrail; /**< Chronological list of recorded audit trail entries. */
    bool echoToConsole;                 /**< Flag indicating whether entries are echoed to std::cout as they arrive. */

public:
    /**
     * @brief Constructs a new AuditLogger instance.
     * @param echo Whether to immediately print each recorded audit entry to standard output (defaults to false).
     */
    explicit AuditLogger(bool echo = false);

    /**
     * @brief Virtual destructor ensuring clean polymorphic destruction.
     */
    virtual ~AuditLogger() override = default;

    /**
     * @brief Observer callback invoked when a workflow lifecycle event occurs.
     * @param e Const reference to the WorkflowEvent payload.
     */
    virtual void onEvent(const WorkflowEvent& e) override;

    /**
     * @brief Formats an event into a standardized, timestamped audit log line.
     * @param e The event to format.
     * @return std::string Formatted audit log entry.
     */
    std::string formatAuditEntry(const WorkflowEvent& e) const;

    /**
     * @brief Retrieves the entire recorded audit trail.
     * @return const std::vector<std::string>& Read-only reference to the audit log entries.
     */
    const std::vector<std::string>& getAuditTrail() const;

    /**
     * @brief Returns the total number of audit records collected.
     * @return size_t Count of entries.
     */
    size_t getEntryCount() const;

    /**
     * @brief Dumps the entire audit trail to standard output.
     */
    void printAuditTrail() const;

    /**
     * @brief Sets whether incoming audit entries are immediately echoed to standard output.
     * @param echo Boolean toggle.
     */
    void setEchoToConsole(bool echo);

    /**
     * @brief Clears all entries from the audit log.
     */
    void clearLog();
};

#endif // AUDIT_LOGGER_H
