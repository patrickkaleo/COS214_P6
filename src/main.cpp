/**
 * @file main.cpp
 * @brief Unit test suite for the Observer and Singleton subsystems.
 * @version 1.0
 */

#include <iostream>
#include <cassert>
#include "EventDispatcher.h"
#include "NotificationService.h"
#include "AuditLogger.h"
#include "MonitoringService.h"

void testSingletonInstance() {
    EventDispatcher& d1 = EventDispatcher::getInstance();
    EventDispatcher& d2 = EventDispatcher::getInstance();
    assert(&d1 == &d2);
    std::cout << "[PASS] testSingletonInstance\n";
}

void testObserverRegistrationAndNotification() {
    EventDispatcher& dispatcher = EventDispatcher::getInstance();
    dispatcher.clearListeners();

    NotificationService notifier("TestChannel", false);
    AuditLogger auditor(false);
    MonitoringService monitor;

    dispatcher.attachListener(&notifier);
    dispatcher.attachListener(&auditor);
    dispatcher.attachListener(&monitor);

    assert(dispatcher.getListenerCount() == 3);

    // Dispatch test events
    dispatcher.notifyListeners(WorkflowEvent(
        EventType::AVAILABLE, "WF-001", "TASK-1", "Verification"
    ));
    dispatcher.notifyListeners(WorkflowEvent(
        EventType::STARTED, "WF-001", "TASK-1", "Verification"
    ));
    dispatcher.notifyListeners(WorkflowEvent(
        EventType::COMPLETED, "WF-001", "TASK-1", "Verification", "Verified"
    ));

    assert(notifier.getNotificationCount() == 3);
    assert(auditor.getEntryCount() == 3);
    assert(monitor.getTotalEvents() == 3);
    assert(monitor.getCountForType(EventType::COMPLETED) == 1);

    dispatcher.clearListeners();
    std::cout << "[PASS] testObserverRegistrationAndNotification\n";
}

void testDynamicDetach() {
    EventDispatcher& dispatcher = EventDispatcher::getInstance();
    dispatcher.clearListeners();

    NotificationService notifier("TestChannel", false);
    AuditLogger auditor(false);

    dispatcher.attachListener(&notifier);
    dispatcher.attachListener(&auditor);
    assert(dispatcher.getListenerCount() == 2);

    dispatcher.notifyListeners(WorkflowEvent(
        EventType::STARTED, "WF-002", "TASK-A", "Step A"
    ));
    assert(notifier.getNotificationCount() == 1);
    assert(auditor.getEntryCount() == 1);

    // Detach notifier
    dispatcher.detachListener(&notifier);
    assert(dispatcher.getListenerCount() == 1);

    // Dispatch next event; notifier should not receive it
    dispatcher.notifyListeners(WorkflowEvent(
        EventType::COMPLETED, "WF-002", "TASK-A", "Step A", "Success"
    ));

    assert(notifier.getNotificationCount() == 1); // count unchanged
    assert(auditor.getEntryCount() == 2);         // auditor received both

    dispatcher.clearListeners();
    std::cout << "[PASS] testDynamicDetach\n";
}

void testAuditTrailFormatting() {
    AuditLogger auditor(false);
    WorkflowEvent e(EventType::ESCALATED, "WF-003", "TASK-B", "Review", "SLA Exceeded");
    
    auditor.onEvent(e);
    assert(auditor.getEntryCount() == 1);

    const std::vector<std::string>& log = auditor.getAuditTrail();
    assert(!log.empty());
    assert(log[0].find("[AUDIT]") != std::string::npos);
    assert(log[0].find("WF-003") != std::string::npos);
    assert(log[0].find("TASK-B") != std::string::npos);
    assert(log[0].find("ESCALATED") != std::string::npos);
    assert(log[0].find("SLA Exceeded") != std::string::npos);

    std::cout << "[PASS] testAuditTrailFormatting\n";
}

void testMonitoringMetricsCalculation() {
    MonitoringService monitor;

    monitor.onEvent(WorkflowEvent(EventType::STARTED, "WF-10", "T1", "Task 1"));
    monitor.onEvent(WorkflowEvent(EventType::COMPLETED, "WF-10", "T1", "Task 1"));
    monitor.onEvent(WorkflowEvent(EventType::FAILED, "WF-10", "T2", "Task 2", "Timeout"));
    monitor.onEvent(WorkflowEvent(EventType::REJECTED, "WF-10", "T3", "Task 3", "Invalid"));

    assert(monitor.getTotalEvents() == 4);
    assert(monitor.getCountForType(EventType::STARTED) == 1);
    assert(monitor.getCountForType(EventType::COMPLETED) == 1);
    assert(monitor.getCountForType(EventType::FAILED) == 1);
    assert(monitor.getCountForType(EventType::REJECTED) == 1);

    // 2 failures out of 4 events = 50.0%
    assert(monitor.getFailureRate() == 50.0);

    monitor.resetMetrics();
    assert(monitor.getTotalEvents() == 0);
    assert(monitor.getFailureRate() == 0.0);

    std::cout << "[PASS] testMonitoringMetricsCalculation\n";
}

void testClearListeners() {
    EventDispatcher& dispatcher = EventDispatcher::getInstance();
    dispatcher.clearListeners();

    NotificationService n1("C1", false);
    NotificationService n2("C2", false);

    dispatcher.attachListener(&n1);
    dispatcher.attachListener(&n2);
    assert(dispatcher.getListenerCount() == 2);

    dispatcher.clearListeners();
    assert(dispatcher.getListenerCount() == 0);

    std::cout << "[PASS] testClearListeners\n";
}

int main() {
    std::cout << "Running unit tests for Observer and Singleton subsystems:\n";

    testSingletonInstance();
    testObserverRegistrationAndNotification();
    testDynamicDetach();
    testAuditTrailFormatting();
    testMonitoringMetricsCalculation();
    testClearListeners();

    std::cout << "All 6 unit tests passed successfully.\n";
    return 0;
}
