/**
 * @file MonitoringService.h
 * @brief Concrete Observer class providing real-time operational telemetry and metrics.
 * @details Implements the WorkflowEventListener interface as part of the Observer pattern.
 * @author Enock Nyamweya
 * @version 2.0
 */

#ifndef MONITORING_SERVICE_H
#define MONITORING_SERVICE_H

#include <map>
#include <set>
#include <string>
#include <iostream>
#include "WorkflowEventListener.h"

/**
 * @class MonitoringService
 * @brief Concrete Observer that tracks statistical metrics, failure rates, and active workflow instances.
 * @details Aggregates real-time event counts across all EventType transitions to provide operational
 *          visibility into throughput, health, and error rates of the generic workflow engine.
 */
class MonitoringService : public WorkflowEventListener {
private:
    size_t totalEventsProcessed;              /**< Total counter of all events observed. */
    std::map<EventType, size_t> eventCounts;  /**< Distribution of events categorized by EventType. */
    std::set<std::string> activeInstances;   /**< Set of workflow instance IDs currently in-flight. */
    std::set<std::string> completedInstances;/**< Set of workflow instance IDs that reached terminal state. */

public:
    /**
     * @brief Constructs a new MonitoringService initializing all counters to zero.
     */
    MonitoringService();

    /**
     * @brief Virtual destructor ensuring clean polymorphic destruction.
     */
    virtual ~MonitoringService() override = default;

    /**
     * @brief Observer callback invoked when a workflow lifecycle event occurs.
     * @param e Const reference to the WorkflowEvent payload.
     */
    virtual void onEvent(const WorkflowEvent& e) override;

    /**
     * @brief Returns the total number of events observed since inception or last reset.
     * @return size_t Total event count.
     */
    size_t getTotalEvents() const;

    /**
     * @brief Returns the count of observed events for a specific EventType.
     * @param type The EventType to query.
     * @return size_t Count of occurrences for that type.
     */
    size_t getCountForType(EventType type) const;

    /**
     * @brief Returns the number of currently active, non-completed workflow instances.
     * @return size_t Active workflow count.
     */
    size_t getActiveInstanceCount() const;

    /**
     * @brief Returns the number of completed workflow instances.
     * @return size_t Completed workflow count.
     */
    size_t getCompletedInstanceCount() const;

    /**
     * @brief Calculates the failure rate as a percentage of total work item transitions.
     * @return double Failure percentage (0.0 to 100.0).
     */
    double getFailureRate() const;

    /**
     * @brief Prints a formatted operational dashboard summary to standard output.
     */
    void printDashboard() const;

    /**
     * @brief Resets all operational telemetry counters and tracked instances.
     */
    void resetMetrics();
};

#endif // MONITORING_SERVICE_H
