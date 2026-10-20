/**
 * @file main.cpp
 * @brief Master doctest test runner for the TIVIDY Generic Workflow Engine.
 *
 * Team Testing Instructions:
 * - This file serves as the unified test driver for all engine subsystems.
 * - Each team member registers their subsystem tests under a dedicated TEST_SUITE:
 *     TEST_SUITE("<Your Name> - <Subsystem Name>") {
 *         TEST_CASE("<Feature or Behavior>") {
 *             CHECK(...);
 *         }
 *     }
 * - Run all tests: make run
 * - Memory leak check: make valgrind
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "EventDispatcher.h"
#include "NotificationService.h"
#include "AuditLogger.h"
#include "MonitoringService.h"

#include "RoutingStrategy.h"
#include "RoleBasedRouting.h"
#include "DataRuleRouting.h"
#include "ApprovalRouting.h"
#include "ApprovalHandler.h"
#include "ApprovalRequest.h"
#include "Participant.h"
#include "ParticipantDirectory.h"
#include "InstanceData.h"

// Temporary test harness helper:
// RoutingStrategy::select() takes a `const Task&` reference.
// To allow Strategy tests to run independently before the Composite subsystem (Task)
// is merged, this provides a safe dummy reference without requiring Task's definition.
class Task;

static const Task& getDummyTask() {
    static const char dummyStorage[64] = {0};
    return *reinterpret_cast<const Task*>(dummyStorage);
}

TEST_SUITE("Enock Nyamweya - Observer & Singleton Subsystems") {

    TEST_CASE("Singleton: verifies single global dispatcher instance") {
        EventDispatcher& d1 = EventDispatcher::getInstance();
        EventDispatcher& d2 = EventDispatcher::getInstance();
        CHECK(&d1 == &d2);
    }

    TEST_CASE("Observer: verifies observer registration and push event dispatching") {
        EventDispatcher& dispatcher = EventDispatcher::getInstance();
        dispatcher.clearListeners();

        NotificationService notifier("TestChannel", false);
        AuditLogger auditor(false);
        MonitoringService monitor;

        dispatcher.attachListener(&notifier);
        dispatcher.attachListener(&auditor);
        dispatcher.attachListener(&monitor);

        CHECK(dispatcher.getListenerCount() == 3);

        dispatcher.notifyListeners(WorkflowEvent(
            EventType::AVAILABLE, "WF-101", "TASK-1", "Registration"
        ));
        dispatcher.notifyListeners(WorkflowEvent(
            EventType::STARTED, "WF-101", "TASK-1", "Registration"
        ));
        dispatcher.notifyListeners(WorkflowEvent(
            EventType::COMPLETED, "WF-101", "TASK-1", "Registration", "Verified"
        ));

        CHECK(notifier.getNotificationCount() == 3);
        CHECK(auditor.getEntryCount() == 3);
        CHECK(monitor.getTotalEvents() == 3);
        CHECK(monitor.getCountForType(EventType::COMPLETED) == 1);

        dispatcher.clearListeners();
    }

    TEST_CASE("Observer: verifies dynamic listener detaching") {
        EventDispatcher& dispatcher = EventDispatcher::getInstance();
        dispatcher.clearListeners();

        NotificationService notifier("Channel", false);
        AuditLogger auditor(false);

        dispatcher.attachListener(&notifier);
        dispatcher.attachListener(&auditor);
        CHECK(dispatcher.getListenerCount() == 2);

        dispatcher.detachListener(&notifier);
        CHECK(dispatcher.getListenerCount() == 1);

        dispatcher.notifyListeners(WorkflowEvent(
            EventType::STARTED, "WF-102", "TASK-2", "Review"
        ));

        CHECK(notifier.getNotificationCount() == 0);
        CHECK(auditor.getEntryCount() == 1);

        dispatcher.clearListeners();
    }

    TEST_CASE("AuditLogger: verifies immutable audit log formatting") {
        AuditLogger auditor(false);
        WorkflowEvent ev(EventType::COMPLETED, "WF-103", "TASK-3", "Verification", "Verified");

        auditor.onEvent(ev);
        CHECK(auditor.getEntryCount() == 1);

        const auto& entries = auditor.getAuditTrail();
        CHECK(entries.size() == 1);
        CHECK(entries[0].find("COMPLETED") != std::string::npos);
        CHECK(entries[0].find("WF-103") != std::string::npos);
        CHECK(entries[0].find("TASK-3") != std::string::npos);
    }

    TEST_CASE("MonitoringService: verifies real-time failure rate calculation") {
        MonitoringService monitor;

        monitor.onEvent(WorkflowEvent(EventType::STARTED, "WF-001", "TASK-1", "Step 1"));
        CHECK(monitor.getActiveInstanceCount() == 1);

        monitor.onEvent(WorkflowEvent(EventType::INSTANCE_COMPLETED, "WF-001", "", "WorkflowInstance", "Success"));
        CHECK(monitor.getActiveInstanceCount() == 0);
        CHECK(monitor.getCompletedInstanceCount() == 1);

        monitor.onEvent(WorkflowEvent(EventType::STARTED, "WF-002", "TASK-1", "Step 1"));
        monitor.onEvent(WorkflowEvent(EventType::FAILED, "WF-002", "TASK-1", "Step 1", "Error"));
        CHECK(monitor.getTotalEvents() == 4);
        CHECK(monitor.getFailureRate() == doctest::Approx(25.0));
    }
}

class MockApprovalHandler : public ApprovalHandler {
private:
    Participant* approver;
public:
    explicit MockApprovalHandler(Participant* p)
        : approver(p) {}

    virtual Participant* handle(ApprovalRequest& req) override {
        (void)req;
        return approver;
    }
};

TEST_SUITE("Enock Nyamweya - Strategy Subsystem") {

    TEST_CASE("RoleBasedRouting: selects first available participant holding required role") {
        ParticipantDirectory directory;
        Participant* alice = new Participant("p1", "Alice Smith", "Triage Nurse");
        Participant* bob = new Participant("p2", "Bob Jones", "Emergency Doctor");
        directory.addParticipant(alice);
        directory.addParticipant(bob);

        RoleBasedRouting strategy("Triage Nurse");
        InstanceData data;

        Participant* selected = strategy.select(getDummyTask(), data, directory);
        REQUIRE(selected != nullptr);
        CHECK(selected->getId() == "p1");
        CHECK(selected->getName() == "Alice Smith");
        CHECK(selected->hasRole("Triage Nurse") == true);
    }

    TEST_CASE("RoleBasedRouting: returns nullptr when no participant holds target role") {
        ParticipantDirectory directory;
        directory.addParticipant(new Participant("p1", "Alice Smith", "Triage Nurse"));

        RoleBasedRouting strategy("Chief Surgeon");
        InstanceData data;

        Participant* selected = strategy.select(getDummyTask(), data, directory);
        CHECK(selected == nullptr);
    }

    TEST_CASE("DataRuleRouting: dynamically maps form data to corresponding roles") {
        ParticipantDirectory directory;
        directory.addParticipant(new Participant("doc1", "Dr. House", "Emergency Doctor"));
        directory.addParticipant(new Participant("doc2", "Dr. Watson", "General Practitioner"));
        directory.addParticipant(new Participant("intern1", "Intern John", "Clinic Intern"));

        std::map<std::string, std::string> mapping = {
            {"critical", "Emergency Doctor"},
            {"mild", "General Practitioner"}
        };
        DataRuleRouting strategy("severity", mapping, "Clinic Intern");

        InstanceData criticalData;
        criticalData.set("severity", "critical");
        Participant* pCritical = strategy.select(getDummyTask(), criticalData, directory);
        REQUIRE(pCritical != nullptr);
        CHECK(pCritical->getId() == "doc1");
        CHECK(pCritical->getName() == "Dr. House");

        InstanceData mildData;
        mildData.set("severity", "mild");
        Participant* pMild = strategy.select(getDummyTask(), mildData, directory);
        REQUIRE(pMild != nullptr);
        CHECK(pMild->getId() == "doc2");
        CHECK(pMild->getName() == "Dr. Watson");
    }

    TEST_CASE("DataRuleRouting: falls back to default role when field is unmapped or missing") {
        ParticipantDirectory directory;
        directory.addParticipant(new Participant("intern1", "Intern John", "Clinic Intern"));

        std::map<std::string, std::string> mapping = {
            {"critical", "Emergency Doctor"}
        };
        DataRuleRouting strategy("severity", mapping, "Clinic Intern");

        InstanceData unknownData;
        unknownData.set("severity", "unknown_value");
        Participant* pUnknown = strategy.select(getDummyTask(), unknownData, directory);
        REQUIRE(pUnknown != nullptr);
        CHECK(pUnknown->getId() == "intern1");

        InstanceData emptyData;
        Participant* pEmpty = strategy.select(getDummyTask(), emptyData, directory);
        REQUIRE(pEmpty != nullptr);
        CHECK(pEmpty->getId() == "intern1");
    }

    TEST_CASE("ApprovalRouting: delegates to ApprovalHandler Chain of Responsibility") {
        Participant* dean = new Participant("p_dean", "Prof. X", "Dean");
        MockApprovalHandler* handler = new MockApprovalHandler(dean);

        ApprovalRouting approvalStrategy(handler);
        InstanceData data;
        ParticipantDirectory directory;

        Participant* approver = approvalStrategy.select(getDummyTask(), data, directory);
        REQUIRE(approver != nullptr);
        CHECK(approver->getId() == "p_dean");
        CHECK(approver->getName() == "Prof. X");

        delete dean;
    }

    TEST_CASE("RoutingStrategy: clone creates independent strategy duplicate") {
        std::map<std::string, std::string> mapping = {
            {"high", "Senior Officer"}
        };
        DataRuleRouting original("priority", mapping, "Junior Officer");

        RoutingStrategy* cloned = original.clone();
        REQUIRE(cloned != nullptr);

        DataRuleRouting* clonedDataRule = dynamic_cast<DataRuleRouting*>(cloned);
        REQUIRE(clonedDataRule != nullptr);
        CHECK(clonedDataRule->getField() == "priority");
        CHECK(clonedDataRule->getFallbackRole() == "Junior Officer");

        clonedDataRule->setFallbackRole("Supervisor");
        CHECK(original.getFallbackRole() == "Junior Officer");
        CHECK(clonedDataRule->getFallbackRole() == "Supervisor");

        delete cloned;
    }
}
