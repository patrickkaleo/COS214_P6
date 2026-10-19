/**
 * @file main.cpp
 * @brief Demonstration driver for the TIVIDY Event & Notification Subsystem (Observer Pattern).
 * @details Conforms to COS 214 Project requirements, testing the EventDispatcher, NotificationService,
 *          AuditLogger, and MonitoringService across diverse workflow event scenarios.
 * @version 1.0
 */

#include <iostream>
#include <memory>
#include "EventDispatcher.h"
#include "NotificationService.h"
#include "AuditLogger.h"
#include "MonitoringService.h"

int main() {
    std::cout << "===========================================================" << std::endl;
    std::cout << "  TIVIDY WORKFLOW MANAGEMENT SYSTEM - OBSERVER SUBSYSTEM  " << std::endl;
    std::cout << "===========================================================\n" << std::endl;

    // 1. Initialize Subject (EventDispatcher)
    EventDispatcher dispatcher;

    // 2. Initialize Concrete Observers
    NotificationService notifier("ConsoleAlerts", true);
    AuditLogger auditor(false); // Do not echo each log line immediately; will dump audit trail at end
    MonitoringService monitor;

    // 3. Register Observers with Subject
    std::cout << ">>> Registering Observers (NotificationService, AuditLogger, MonitoringService)..." << std::endl;
    dispatcher.attachListener(&notifier);
    dispatcher.attachListener(&auditor);
    dispatcher.attachListener(&monitor);
    std::cout << "Active listeners attached: " << dispatcher.getListenerCount() << "\n" << std::endl;

    // 4. Simulate Workflow Instance WF-101 Execution
    std::cout << ">>> [Scenario 1] Executing Workflow WF-101 (Standard Process with Escalation)..." << std::endl;
    
    // Step 1: Document Ingestion
    dispatcher.notifyListeners(WorkflowEvent(
        EventType::AVAILABLE, "WF-101", "STEP-01", "Document Intake"
    ));

    dispatcher.notifyListeners(WorkflowEvent(
        EventType::ASSIGNED, "WF-101", "STEP-01", "Document Intake", "Assigned to Agent_Smith"
    ));

    dispatcher.notifyListeners(WorkflowEvent(
        EventType::STARTED, "WF-101", "STEP-01", "Document Intake"
    ));

    dispatcher.notifyListeners(WorkflowEvent(
        EventType::COMPLETED, "WF-101", "STEP-01", "Document Intake", "Payload verified successfully"
    ));

    // Step 2: Policy Compliance Check with Escalation
    dispatcher.notifyListeners(WorkflowEvent(
        EventType::AVAILABLE, "WF-101", "STEP-02", "Compliance Review"
    ));

    dispatcher.notifyListeners(WorkflowEvent(
        EventType::ESCALATED, "WF-101", "STEP-02", "Compliance Review", "SLA threshold exceeded (48h)"
    ));

    dispatcher.notifyListeners(WorkflowEvent(
        EventType::COMPLETED, "WF-101", "STEP-02", "Compliance Review", "Approved by Executive Committee"
    ));

    // Root instance completion
    dispatcher.notifyListeners(WorkflowEvent(
        EventType::INSTANCE_COMPLETED, "WF-101", "ROOT", "Workflow Root", "All stages completed"
    ));

    std::cout << "\n>>> [Scenario 2] Executing Workflow WF-102 (Failure Scenario)..." << std::endl;
    dispatcher.notifyListeners(WorkflowEvent(
        EventType::STARTED, "WF-102", "STEP-01", "External Payment Gateway"
    ));

    dispatcher.notifyListeners(WorkflowEvent(
        EventType::FAILED, "WF-102", "STEP-01", "External Payment Gateway", "HTTP 504 Gateway Timeout"
    ));

    // 5. Demonstrate Dynamic Observer Detachment (Unsubscribe)
    std::cout << "\n>>> [Scenario 3] Dynamic Detach Test: Unsubscribing NotificationService..." << std::endl;
    dispatcher.detachListener(&notifier);
    std::cout << "Active listeners attached after detach: " << dispatcher.getListenerCount() << std::endl;

    std::cout << "Dispatching new event (NotificationService should NOT receive this)..." << std::endl;
    dispatcher.notifyListeners(WorkflowEvent(
        EventType::STARTED, "WF-103", "STEP-01", "Background Archival"
    ));

    std::cout << "\n";

    // 6. Inspect Audit Trail
    auditor.printAuditTrail();

    std::cout << "\n";

    // 7. Inspect Operational Metrics & Monitoring Dashboard
    monitor.printDashboard();

    std::cout << "\n>>> Total notifications logged by NotificationService: " 
              << notifier.getNotificationCount() << std::endl;
    std::cout << ">>> Total audit records captured: " 
              << auditor.getEntryCount() << std::endl;
    std::cout << ">>> Total events monitored: " 
              << monitor.getTotalEvents() << std::endl;

    std::cout << "\n===========================================================" << std::endl;
    std::cout << "  OBSERVER SUBSYSTEM DEMONSTRATION COMPLETE - CLEAN EXIT   " << std::endl;
    std::cout << "===========================================================" << std::endl;

    return 0;
}
