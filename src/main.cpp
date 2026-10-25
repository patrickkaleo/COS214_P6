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

#include "WorkItem.h"
#include "WorkItemState.h"

#include "ExternalService.h"
#include "ServiceRegistry.h"
#include "GovernmentIDAdapter.h"
#include "BankGatewayAdapter.h"
#include "TrackingServiceAdapter.h"
#include "CarrierAdapter.h"

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

// ---------------------------------------------------------------------------
// State pattern test helpers
// ---------------------------------------------------------------------------

/// Minimal concrete WorkItem so the lifecycle can be tested before Task/SubProcess land.
class LifecycleTestItem : public WorkItem {
public:
    int availableHookCalls;

    LifecycleTestItem(const std::string& id, const std::string& itemName)
        : WorkItem(id, itemName), availableHookCalls(0) {}

    WorkItem* clone() const override { return new LifecycleTestItem(*this); }

protected:
    void onAvailable() override { ++availableHookCalls; }
};

/// Records the event types published by work items through the EventDispatcher.
class EventRecorder : public WorkflowEventListener {
public:
    std::vector<WorkflowEvent> events;
    void onEvent(const WorkflowEvent& e) override { events.push_back(e); }
};

/// Moves a fresh item to InProgress, assigned to @p p.
static void driveToInProgress(WorkItem& item, Participant* p) {
    REQUIRE(item.makeAvailable());
    REQUIRE(item.assign(p));
    REQUIRE(item.start());
}

TEST_SUITE("Angela Ramaboea - State Subsystem") {

    TEST_CASE("State: a new work item starts in Created and is not finished") {
        LifecycleTestItem item("T1", "Faculty Review");
        CHECK(item.getStateName() == "Created");
        CHECK_FALSE(item.isFinished());
        CHECK(item.getAssignee() == nullptr);
    }

    TEST_CASE("State: happy path Created -> Available -> Assigned -> InProgress -> Completed") {
        Participant officer("P1", "Thandi", "FacultyOfficer");
        LifecycleTestItem item("T1", "Faculty Review");

        CHECK(item.makeAvailable());
        CHECK(item.getStateName() == "Available");
        CHECK(item.availableHookCalls == 1);

        CHECK(item.assign(&officer));
        CHECK(item.getStateName() == "Assigned");
        CHECK(item.getAssignee() == &officer);

        CHECK(item.start());
        CHECK(item.getStateName() == "InProgress");

        CHECK(item.complete("approved"));
        CHECK(item.getStateName() == "Completed");
        CHECK(item.getOutcome() == "approved");
        CHECK(item.isFinished());
    }

    TEST_CASE("State: rejection records the reason and is final") {
        Participant dean("P2", "Dr Mokoena", "Dean");
        LifecycleTestItem item("T2", "Admission Decision");
        driveToInProgress(item, &dean);

        CHECK(item.reject("APS below faculty minimum"));
        CHECK(item.getStateName() == "Rejected");
        CHECK(item.getOutcome() == "APS below faculty minimum");
        CHECK(item.isFinished());
    }

    TEST_CASE("State: invalid transitions are refused and leave the state unchanged") {
        Participant clerk("P3", "Sipho", "WarehouseClerk");
        LifecycleTestItem item("T3", "Log Exception");

        // Created: cannot be assigned, started, completed, rejected or escalated
        CHECK_FALSE(item.assign(&clerk));
        CHECK_FALSE(item.start());
        CHECK_FALSE(item.complete("done"));
        CHECK_FALSE(item.reject("no"));
        CHECK_FALSE(item.escalate());
        CHECK(item.getStateName() == "Created");

        // Available: cannot start or complete before someone is assigned
        REQUIRE(item.makeAvailable());
        CHECK_FALSE(item.makeAvailable());
        CHECK_FALSE(item.start());
        CHECK_FALSE(item.complete("done"));
        CHECK(item.getStateName() == "Available");

        // Assigned: cannot complete without starting
        REQUIRE(item.assign(&clerk));
        CHECK_FALSE(item.complete("done"));
        CHECK(item.getStateName() == "Assigned");
    }

    TEST_CASE("State: assigning a null participant is refused") {
        LifecycleTestItem item("T4", "Classify Severity");
        REQUIRE(item.makeAvailable());
        CHECK_FALSE(item.assign(nullptr));
        CHECK(item.getStateName() == "Available");
    }

    TEST_CASE("State: final states refuse every request") {
        Participant p("P5", "Lerato", "Dispatcher");
        LifecycleTestItem item("T5", "Reroute Shipment");
        driveToInProgress(item, &p);
        REQUIRE(item.complete("rerouted"));

        CHECK_FALSE(item.makeAvailable());
        CHECK_FALSE(item.assign(&p));
        CHECK_FALSE(item.start());
        CHECK_FALSE(item.complete("again"));
        CHECK_FALSE(item.reject("late"));
        CHECK_FALSE(item.cancel());
        CHECK_FALSE(item.escalate());
        CHECK(item.getStateName() == "Completed");
        CHECK(item.getOutcome() == "rerouted");
    }

    TEST_CASE("State: escalation releases the assignee and re-assignment resumes work") {
        Participant officer("P6", "Naledi", "FinancialAidOfficer");
        Participant manager("P7", "Mr Botha", "FinancialAidManager");
        LifecycleTestItem item("T6", "Approve Bursary");
        driveToInProgress(item, &officer);

        CHECK(item.escalate());
        CHECK(item.getStateName() == "Escalated");
        CHECK(item.getAssignee() == nullptr);
        CHECK(item.getEscalationLevel() == 1);

        // Escalated work must be re-assigned before it can continue
        CHECK_FALSE(item.start());
        CHECK_FALSE(item.complete("approved"));
        CHECK(item.assign(&manager));
        CHECK(item.getStateName() == "Assigned");
        CHECK(item.getAssignee() == &manager);

        CHECK(item.start());
        CHECK(item.complete("approved"));
        CHECK(item.isFinished());
    }

    TEST_CASE("State: delegation re-assigns an Assigned item to another participant") {
        Participant a("P8", "Kabelo", "Registrar");
        Participant b("P9", "Zanele", "Registrar");
        LifecycleTestItem item("T7", "Verify Documents");
        REQUIRE(item.makeAvailable());
        REQUIRE(item.assign(&a));

        CHECK(item.assign(&b));
        CHECK(item.getStateName() == "Assigned");
        CHECK(item.getAssignee() == &b);
    }

    TEST_CASE("State: cancellation is allowed from every non-final state") {
        Participant p("P10", "Ayanda", "CustomerService");

        LifecycleTestItem created("C1", "Created item");
        CHECK(created.cancel());
        CHECK(created.getStateName() == "Cancelled");
        CHECK(created.isFinished());

        LifecycleTestItem available("C2", "Available item");
        REQUIRE(available.makeAvailable());
        CHECK(available.cancel());

        LifecycleTestItem inProgress("C3", "In-progress item");
        driveToInProgress(inProgress, &p);
        CHECK(inProgress.cancel());

        LifecycleTestItem escalated("C4", "Escalated item");
        REQUIRE(escalated.makeAvailable());
        REQUIRE(escalated.escalate());
        CHECK(escalated.cancel());
        CHECK(escalated.getStateName() == "Cancelled");
    }

    TEST_CASE("State: each transition publishes the matching event through the dispatcher") {
        EventDispatcher& dispatcher = EventDispatcher::getInstance();
        dispatcher.clearListeners();
        EventRecorder recorder;
        dispatcher.attachListener(&recorder);

        Participant p("P11", "Musa", "Dean");
        LifecycleTestItem item("T8", "Admission Decision");
        item.setInstanceId("UNI-001");
        driveToInProgress(item, &p);
        item.complete("approved");
        item.start(); // refused, must not publish

        REQUIRE(recorder.events.size() == 4);
        CHECK(recorder.events[0].type == EventType::AVAILABLE);
        CHECK(recorder.events[1].type == EventType::ASSIGNED);
        CHECK(recorder.events[2].type == EventType::STARTED);
        CHECK(recorder.events[3].type == EventType::COMPLETED);
        CHECK(recorder.events[3].outcome == "approved");
        CHECK(recorder.events[3].instanceId == "UNI-001");
        CHECK(recorder.events[3].itemId == "T8");

        dispatcher.clearListeners();
    }

    TEST_CASE("State: a cloned item has its own independent state") {
        Participant p("P12", "Palesa", "Registrar");
        LifecycleTestItem original("T9", "Registration");
        REQUIRE(original.makeAvailable());

        WorkItem* copy = original.clone();
        CHECK(copy->getStateName() == "Available");

        REQUIRE(copy->assign(&p));
        CHECK(copy->getStateName() == "Assigned");
        CHECK(original.getStateName() == "Available");

        delete copy;
    }
}

// ---------------------------------------------------------------------------
// Adapter pattern tests
// ---------------------------------------------------------------------------

/// Builds a request from key/value pairs, as a task would from its InstanceData.
static ServiceRequest makeRequest(const std::string& op,
                                  const std::map<std::string, std::string>& params) {
    return ServiceRequest(op, params);
}

TEST_SUITE("Angela Ramaboea - Adapter Subsystem") {

    TEST_CASE("Adapter: registry resolves adapters by the names used in workflow definitions") {
        ServiceRegistry registry;
        registry.add(new GovernmentIDAdapter());
        registry.add(new BankGatewayAdapter());
        registry.add(new TrackingServiceAdapter());
        registry.add(new CarrierAdapter());

        CHECK(registry.size() == 4);
        CHECK(registry.has("DocumentVerificationService"));
        CHECK(registry.has("PaymentGateway"));
        CHECK(registry.has("TrackingService"));
        CHECK(registry.has("CarrierAdapter"));
        CHECK(registry.find("NoSuchService") == nullptr);

        // The engine only ever sees the Target interface
        ExternalService* svc = registry.find("TrackingService");
        REQUIRE(svc != nullptr);
        CHECK(svc->name() == "TrackingService");
    }

    TEST_CASE("Adapter: registering a service under an existing name replaces it") {
        ServiceRegistry registry;
        GovernmentIDAdapter* first = new GovernmentIDAdapter();
        GovernmentIDAdapter* second = new GovernmentIDAdapter();
        registry.add(first);
        registry.add(second);
        registry.add(nullptr);

        CHECK(registry.size() == 1);
        CHECK(registry.find("DocumentVerificationService") == second);
    }

    TEST_CASE("Adapter: GovernmentIDAdapter verifies a matching identity") {
        GovernmentIDAdapter adapter;
        ServiceResult r = adapter.invoke(makeRequest("verifyIdentity",
            {{"idNumber", "0501015800087"}, {"surname", "mokoena"}}));

        CHECK(r.success);
        CHECK(r.outputs["identityVerified"] == "true");
        CHECK(r.outputs["identityStatus"] == "VERIFIED");
    }

    TEST_CASE("Adapter: GovernmentIDAdapter reports negative answers as data, not failures") {
        GovernmentIDAdapter adapter;

        ServiceResult mismatch = adapter.invoke(makeRequest("verifyIdentity",
            {{"idNumber", "0501015800087"}, {"surname", "Naidoo"}}));
        CHECK(mismatch.success);
        CHECK(mismatch.outputs["identityVerified"] == "false");
        CHECK(mismatch.outputs["identityStatus"] == "SURNAME_MISMATCH");

        ServiceResult unknown = adapter.invoke(makeRequest("verifyIdentity",
            {{"idNumber", "9999999999999"}, {"surname", "Smith"}}));
        CHECK(unknown.outputs["identityStatus"] == "NOT_FOUND");

        ServiceResult deceased = adapter.invoke(makeRequest("verifyIdentity",
            {{"idNumber", "4808105009083"}, {"surname", "Dlamini"}}));
        CHECK(deceased.outputs["identityStatus"] == "DECEASED");
    }

    TEST_CASE("Adapter: GovernmentIDAdapter fails on bad input or registry outage") {
        GovernmentIDAdapter adapter;

        CHECK_FALSE(adapter.invoke(makeRequest("verifyIdentity", {{"surname", "Mokoena"}})).success);
        CHECK_FALSE(adapter.invoke(makeRequest("verifyIdentity",
            {{"idNumber", "12AB"}, {"surname", "Mokoena"}})).success);

        adapter.getRegistry().setOnline(false);
        ServiceResult down = adapter.invoke(makeRequest("verifyIdentity",
            {{"idNumber", "0501015800087"}, {"surname", "Mokoena"}}));
        CHECK_FALSE(down.success);
        CHECK(down.message == "Identity registry unavailable");
    }

    TEST_CASE("Adapter: BankGatewayAdapter converts rands to an EFT and returns a reference") {
        BankGatewayAdapter adapter;
        ServiceResult r = adapter.invoke(makeRequest("pay",
            {{"accountNumber", "62012345678"}, {"branchCode", "250655"},
             {"amount", "45000.50"}, {"reference", "BURSARY-UNI-001"}}));

        CHECK(r.success);
        CHECK(r.outputs["paymentStatus"] == "PAID");
        CHECK(r.outputs["paymentReference"].find("EFT") == 0);
    }

    TEST_CASE("Adapter: BankGatewayAdapter maps bank rejections to failures") {
        BankGatewayAdapter adapter(new BankPaymentGateway(5000000LL)); // R50 000 limit

        ServiceResult overLimit = adapter.invoke(makeRequest("pay",
            {{"accountNumber", "62012345678"}, {"branchCode", "250655"}, {"amount", "60000"}}));
        CHECK_FALSE(overLimit.success);
        CHECK(overLimit.message == "Amount exceeds the gateway transfer limit");

        ServiceResult badAccount = adapter.invoke(makeRequest("pay",
            {{"accountNumber", "12"}, {"branchCode", "250655"}, {"amount", "100"}}));
        CHECK_FALSE(badAccount.success);

        ServiceResult badAmount = adapter.invoke(makeRequest("pay",
            {{"accountNumber", "62012345678"}, {"branchCode", "250655"}, {"amount", "ten"}}));
        CHECK_FALSE(badAmount.success);

        adapter.getGateway().setOnline(false);
        ServiceResult down = adapter.invoke(makeRequest("pay",
            {{"accountNumber", "62012345678"}, {"branchCode", "250655"}, {"amount", "100"}}));
        CHECK_FALSE(down.success);
        CHECK(down.message == "Bank gateway unavailable");
    }

    TEST_CASE("Adapter: TrackingServiceAdapter parses the courier reply into named data") {
        TrackingServiceAdapter adapter;
        ServiceResult r = adapter.invoke(makeRequest("track", {{"waybill", "WB1002"}}));

        CHECK(r.success);
        CHECK(r.outputs["shipmentStatus"] == "DELAYED");
        CHECK(r.outputs["lastLocation"] == "Bloemfontein Depot");
        CHECK(r.outputs["delayHours"] == "9");

        adapter.getApi().registerParcel("WB2000", "IN_TRANSIT", "Polokwane", 61);
        CHECK(adapter.invoke(makeRequest("track", {{"waybill", "WB2000"}})).outputs["delayHours"] == "2");
    }

    TEST_CASE("Adapter: TrackingServiceAdapter fails for unknown waybills and outages") {
        TrackingServiceAdapter adapter;
        CHECK_FALSE(adapter.invoke(makeRequest("track", {{"waybill", "WB404"}})).success);
        CHECK_FALSE(adapter.invoke(makeRequest("track", {})).success);

        adapter.getApi().setOnline(false);
        ServiceResult down = adapter.invoke(makeRequest("track", {{"waybill", "WB1001"}}));
        CHECK_FALSE(down.success);
        CHECK(down.message == "Courier tracking service unavailable");
    }

    TEST_CASE("Adapter: CarrierAdapter maps hub names to codes and returns the confirmation") {
        CarrierAdapter adapter;
        ServiceResult r = adapter.invoke(makeRequest("reroute",
            {{"waybill", "WB1003"}, {"destinationHub", "Cape Town"}}));

        CHECK(r.success);
        CHECK(r.outputs["newHub"] == "Cape Town");
        CHECK(r.outputs["rerouteReference"] == "RB21-WB1003-001");
    }

    TEST_CASE("Adapter: CarrierAdapter fails for unknown hubs, full hubs and outages") {
        CarrierAdapter adapter;
        CHECK_FALSE(adapter.invoke(makeRequest("reroute",
            {{"waybill", "WB1003"}, {"destinationHub", "Atlantis"}})).success);

        adapter.getCarrier().setHubFull(CarrierBookingSystem::HUB_DURBAN, true);
        ServiceResult full = adapter.invoke(makeRequest("reroute",
            {{"waybill", "WB1003"}, {"destinationHub", "Durban"}}));
        CHECK_FALSE(full.success);
        CHECK(full.message == "No carrier capacity at Durban");

        adapter.getCarrier().setOnline(false);
        CHECK_FALSE(adapter.invoke(makeRequest("reroute",
            {{"waybill", "WB1003"}, {"destinationHub", "Johannesburg"}})).success);
    }

    TEST_CASE("Adapter: service outputs are written into the instance data") {
        InstanceData data;
        data.set("waybill", "WB1001");

        TrackingServiceAdapter adapter;
        ServiceResult r = adapter.invoke(ServiceRequest("track", data.getMap()));
        REQUIRE(r.success);
        r.applyTo(data);

        CHECK(data.get("shipmentStatus") == "IN_TRANSIT");
        CHECK(data.get("lastLocation") == "Durban Hub");
        CHECK(data.get("waybill") == "WB1001");
    }
}
