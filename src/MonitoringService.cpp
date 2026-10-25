/**
 * @file MonitoringService.cpp
 * @brief Implementation of the MonitoringService concrete Observer class.
 * @version 1.0
 */

#include "MonitoringService.h"
#include <iomanip>

MonitoringService::MonitoringService()
    : totalEventsProcessed(0) {
}

void MonitoringService::onEvent(const WorkflowEvent& e) {
    totalEventsProcessed++;
    eventCounts[e.type]++;

    if (!e.instanceId.empty()) {
        if (e.type == EventType::INSTANCE_COMPLETED) {
            activeInstances.erase(e.instanceId);
            completedInstances.insert(e.instanceId);
        } else if (e.type == EventType::FAILED || e.type == EventType::CANCELLED) {
            // If the failure was an entire instance failure, remove from active
            if (e.itemId.empty() || e.itemId == e.instanceId) {
                activeInstances.erase(e.instanceId);
            }
        } else {
            // Any initial transition marks the instance active unless already completed
            if (completedInstances.find(e.instanceId) == completedInstances.end()) {
                activeInstances.insert(e.instanceId);
            }
        }
    }
}

size_t MonitoringService::getTotalEvents() const {
    return totalEventsProcessed;
}

size_t MonitoringService::getCountForType(EventType type) const {
    auto it = eventCounts.find(type);
    if (it != eventCounts.end()) {
        return it->second;
    }
    return 0;
}

size_t MonitoringService::getActiveInstanceCount() const {
    return activeInstances.size();
}

size_t MonitoringService::getCompletedInstanceCount() const {
    return completedInstances.size();
}

double MonitoringService::getFailureRate() const {
    if (totalEventsProcessed == 0) {
        return 0.0;
    }
    size_t failures = getCountForType(EventType::FAILED) + getCountForType(EventType::REJECTED);
    return (static_cast<double>(failures) / static_cast<double>(totalEventsProcessed)) * 100.0;
}

void MonitoringService::printDashboard() const {
    std::cout << "Monitoring Dashboard Metrics:\n";
    std::cout << "  Total Events:      " << totalEventsProcessed << "\n";
    std::cout << "  Active Workflows:  " << activeInstances.size() << "\n";
    std::cout << "  Completed:         " << completedInstances.size() << "\n";
    std::cout << "  Failure Rate:      " << std::fixed << std::setprecision(2) << getFailureRate() << "%\n";
    std::cout << "  Event Breakdown:\n";

    const EventType allTypes[] = {
        EventType::AVAILABLE,
        EventType::ASSIGNED,
        EventType::STARTED,
        EventType::COMPLETED,
        EventType::REJECTED,
        EventType::CANCELLED,
        EventType::ESCALATED,
        EventType::FAILED,
        EventType::INSTANCE_COMPLETED
    };

    for (EventType t : allTypes) {
        std::cout << "    " << std::left << std::setw(20) << eventTypeToString(t) 
                  << ": " << getCountForType(t) << "\n";
    }
}

void MonitoringService::resetMetrics() {
    totalEventsProcessed = 0;
    eventCounts.clear();
    activeInstances.clear();
    completedInstances.clear();
}
