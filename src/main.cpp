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
#include "Task.h"
#include "SubProcess.h"
#include "Iterator.h"
#include "WorkItemIterator.h"
#include "DepthFirstWorkItemIterator.h"
#include "WorkflowMediator.h"
#include "DependencyMediator.h"
#include "DependencyRule.h"
#include "ConcreteApprovalHandler.h"
#include "ApprovalChainIterator.h"
#include "ThresholdRule.h"
#include "AlwaysAcceptRule.h"

#include "ExternalService.h"
#include "ServiceRegistry.h"
#include "GovernmentIDAdapter.h"
#include "BankGatewayAdapter.h"
#include "TrackingServiceAdapter.h"
#include "CarrierAdapter.h"

// RoutingStrategy::select() takes a `const Task&`; strategies under test that ignore
// the task still need a real one (ApprovalRouting reads its escalation level).
static const Task& getDummyTask() {
    static const Task dummy("dummy", "Dummy Task");
    return dummy;
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

    virtual ApprovalHandler* clone() const override {
        return new MockApprovalHandler(approver);
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

// ---------------------------------------------------------------------------
// Composite, Iterator, Mediator and Chain of Responsibility test helpers
// ---------------------------------------------------------------------------

/// Task routed to the first participant holding @p role.
static Task* roleTask(const std::string& id, const std::string& name, const std::string& role) {
    Task* task = new Task(id, name);
    task->setRouting(new RoleBasedRouting(role));
    return task;
}

/// Starts and completes an item that routing has already assigned.
static bool finishWith(WorkItem* item, const std::string& outcome) {
    return item != nullptr && item->start() && item->complete(outcome);
}

/// Ids visited by a work-item iterator, in order.
static std::vector<std::string> visitOrder(Iterator<WorkItem>& it) {
    std::vector<std::string> ids;
    for (it.first(); !it.isDone(); it.next()) {
        ids.push_back(it.current()->getId());
    }
    return ids;
}

/// Shipment-exception process: report, then handle (reroute or hold), then close.
static SubProcess* buildShipmentExceptionTree() {
    SubProcess* root = new SubProcess("SHIP_EX", "Shipment Exception");

    SubProcess* report = new SubProcess("REPORT", "Exception Report");
    report->add(roleTask("LOG_EXCEPTION", "Log Exception", "WarehouseClerk"));
    report->add(roleTask("CLASSIFY", "Classify Severity", "ExceptionAnalyst"));

    SubProcess* handle = new SubProcess("HANDLE", "Exception Handling");
    handle->add(roleTask("ASSESS", "Assess Impact", "LogisticsCoordinator"));
    handle->add(roleTask("REROUTE", "Reroute Shipment", "Dispatcher"));
    handle->add(roleTask("HOLD", "Hold Shipment", "WarehouseSupervisor"));

    SubProcess* close = new SubProcess("CLOSE", "Case Closure");
    close->add(roleTask("NOTIFY_CUSTOMER", "Notify Customer", "CustomerService"));
    close->add(roleTask("CLOSE_CASE", "Close Case", "LogisticsCoordinator"));

    root->add(report);
    root->add(handle);
    root->add(close);
    return root;
}

static std::vector<DependencyRule> shipmentExceptionRules() {
    std::vector<DependencyRule> rules;
    rules.push_back(DependencyRule("REPORT", "HANDLE", "*"));
    rules.push_back(DependencyRule("HANDLE", "CLOSE", "*"));
    rules.push_back(DependencyRule("LOG_EXCEPTION", "CLASSIFY", "*"));
    rules.push_back(DependencyRule("ASSESS", "REROUTE", "reroute"));
    rules.push_back(DependencyRule("ASSESS", "HOLD", "hold"));
    rules.push_back(DependencyRule("NOTIFY_CUSTOMER", "CLOSE_CASE", "*"));
    return rules;
}

static void addLogisticsStaff(ParticipantDirectory& directory) {
    directory.addParticipant(new Participant("L1", "Sipho Ndlovu", "WarehouseClerk"));
    directory.addParticipant(new Participant("L2", "Ayesha Patel", "ExceptionAnalyst"));
    directory.addParticipant(new Participant("L3", "Johan van Wyk", "LogisticsCoordinator"));
    directory.addParticipant(new Participant("L4", "Lerato Mokoena", "Dispatcher"));
    directory.addParticipant(new Participant("L5", "Pieter Botha", "WarehouseSupervisor"));
    directory.addParticipant(new Participant("L6", "Nomsa Khumalo", "CustomerService"));
}

/// One running instance: its own copy of the tree, its own mediator and its own data.
struct TestInstance {
    std::unique_ptr<SubProcess> root;
    DependencyMediator mediator;
    InstanceData data;

    TestInstance(const SubProcess& definitionRoot,
                 const std::vector<DependencyRule>& rules,
                 const ParticipantDirectory& directory)
        : root(definitionRoot.clone()), mediator(rules) {
        mediator.registerTree(*root);
        root->setRoutingContext(&data, &directory);
    }

    WorkItem* item(const std::string& id) { return mediator.findItem(id); }
    std::string state(const std::string& id) { return item(id)->getStateName(); }
};

/// Bursary approval levels: officer up to R20 000, manager up to R100 000, then committee.
static ApprovalHandler* buildBursaryChain() {
    ApprovalHandler* officer = new ConcreteApprovalHandler("BursaryOfficer", new ThresholdRule("amount", 20000), 48);
    officer->setNext(new ConcreteApprovalHandler("FinanceManager", new ThresholdRule("amount", 100000), 72))
           ->setNext(new ConcreteApprovalHandler("BursaryCommittee", new AlwaysAcceptRule(), 168));
    return officer;
}

static void addBursaryApprovers(ParticipantDirectory& directory) {
    directory.addParticipant(new Participant("B1", "Zanele Dube", "BursaryOfficer"));
    directory.addParticipant(new Participant("B2", "Michael Smith", "FinanceManager"));
    directory.addParticipant(new Participant("B3", "Prof. Naidoo", "BursaryCommittee"));
}

TEST_SUITE("Angelo Anthony - Composite & Iterator Subsystem") {

    TEST_CASE("Iterator: depth-first traversal visits parents before children, left to right") {
        std::unique_ptr<SubProcess> root(buildShipmentExceptionTree());
        std::unique_ptr<WorkItemIterator> it = root->createIterator();

        std::vector<std::string> expected = {
            "SHIP_EX", "REPORT", "LOG_EXCEPTION", "CLASSIFY",
            "HANDLE", "ASSESS", "REROUTE", "HOLD",
            "CLOSE", "NOTIFY_CUSTOMER", "CLOSE_CASE"};
        CHECK(visitOrder(*it) == expected);
    }

    TEST_CASE("Iterator: first() restarts, and a finished iterator stays finished") {
        std::unique_ptr<SubProcess> root(buildShipmentExceptionTree());
        std::unique_ptr<WorkItemIterator> it = root->createIterator();

        while (!it->isDone()) {
            it->next();
        }
        CHECK(it->current() == nullptr);
        it->next(); // harmless once done
        CHECK(it->isDone());

        it->first();
        REQUIRE_FALSE(it->isDone());
        CHECK(it->current()->getId() == "SHIP_EX");
    }

    TEST_CASE("Iterator: a single task and an empty root are traversed correctly") {
        Task task("T1", "Capture Application");
        DepthFirstWorkItemIterator single(&task);
        CHECK(visitOrder(single) == std::vector<std::string>{"T1"});

        DepthFirstWorkItemIterator empty(nullptr);
        CHECK(empty.isDone());
        CHECK(empty.current() == nullptr);
    }

    TEST_CASE("Composite: find() locates nested items and parents are linked") {
        std::unique_ptr<SubProcess> root(buildShipmentExceptionTree());

        WorkItem* hold = root->find("HOLD");
        REQUIRE(hold != nullptr);
        CHECK(hold->getName() == "Hold Shipment");
        REQUIRE(hold->getParent() != nullptr);
        CHECK(hold->getParent()->getId() == "HANDLE");
        CHECK(hold->getParent()->getParent() == root.get());

        CHECK(root->find("SHIP_EX") == root.get());
        CHECK(root->find("NOT_THERE") == nullptr);
        CHECK(hold->childCount() == 0);
        CHECK(hold->getChild(0) == nullptr);
    }

    TEST_CASE("Composite: remove() deletes only the named direct child") {
        SubProcess handle("HANDLE", "Exception Handling");
        handle.add(new Task("ASSESS", "Assess Impact"));
        handle.add(new Task("HOLD", "Hold Shipment"));
        handle.add(nullptr); // ignored

        CHECK(handle.childCount() == 2);
        CHECK(handle.remove("ASSESS"));
        CHECK_FALSE(handle.remove("ASSESS"));
        CHECK(handle.childCount() == 1);
        CHECK(handle.getChild(0)->getId() == "HOLD");
        CHECK(handle.getChild(5) == nullptr);
    }

    TEST_CASE("Composite: clone() deep-copies the tree so instances are isolated") {
        std::unique_ptr<SubProcess> definition(buildShipmentExceptionTree());
        std::unique_ptr<SubProcess> copy(definition->clone());

        WorkItem* original = definition->find("ASSESS");
        WorkItem* copied = copy->find("ASSESS");
        REQUIRE(copied != nullptr);
        CHECK(copied != original);
        CHECK(copied->getParent() == copy->find("HANDLE"));

        Task* copiedTask = static_cast<Task*>(copied);
        Task* originalTask = static_cast<Task*>(original);
        REQUIRE(copiedTask->getRouting() != nullptr);
        CHECK(copiedTask->getRouting() != originalTask->getRouting());

        // Progress in the copy must not leak into the definition.
        CHECK(copied->makeAvailable());
        CHECK(copied->getStateName() == "Available");
        CHECK(original->getStateName() == "Created");

        CHECK(copy->remove("REPORT"));
        CHECK(definition->find("LOG_EXCEPTION") != nullptr);
    }

    TEST_CASE("Iterator: approval chain iterator lists every level in order") {
        std::unique_ptr<ApprovalHandler> chain(buildBursaryChain());
        std::unique_ptr<Iterator<ApprovalHandler>> it = chain->createIterator();

        std::vector<std::string> levels;
        for (it->first(); !it->isDone(); it->next()) {
            levels.push_back(it->current()->describe());
        }
        REQUIRE(levels.size() == 3);
        CHECK(levels[0] == "BursaryOfficer (amount <= 20000)");
        CHECK(levels[1] == "FinanceManager (amount <= 100000)");
        CHECK(levels[2] == "BursaryCommittee (any request)");

        ApprovalChainIterator empty(nullptr);
        CHECK(empty.isDone());
    }
}

TEST_SUITE("Angelo Anthony - Mediator Subsystem") {

    TEST_CASE("Mediator: registerTree links every item to the mediator") {
        std::unique_ptr<SubProcess> root(buildShipmentExceptionTree());
        DependencyMediator mediator(shipmentExceptionRules());
        mediator.registerTree(*root);

        std::unique_ptr<WorkItemIterator> it = root->createIterator();
        int count = 0;
        for (it->first(); !it->isDone(); it->next()) {
            CHECK(it->current()->getMediator() == &mediator);
            CHECK(mediator.findItem(it->current()->getId()) == it->current());
            ++count;
        }
        CHECK(count == 11);
        CHECK(mediator.findItem("UNKNOWN") == nullptr);
    }

    TEST_CASE("Mediator: starting the root opens only entry items, and routing assigns them") {
        ParticipantDirectory directory;
        addLogisticsStaff(directory);
        std::unique_ptr<SubProcess> definition(buildShipmentExceptionTree());
        TestInstance inst(*definition, shipmentExceptionRules(), directory);

        REQUIRE(inst.root->makeAvailable());
        CHECK(inst.state("SHIP_EX") == "InProgress");
        CHECK(inst.state("REPORT") == "InProgress");
        CHECK(inst.state("LOG_EXCEPTION") == "Assigned");
        CHECK(inst.item("LOG_EXCEPTION")->getAssignee()->getName() == "Sipho Ndlovu");
        CHECK(inst.state("CLASSIFY") == "Created");   // waits for LOG_EXCEPTION
        CHECK(inst.state("HANDLE") == "Created");     // waits for REPORT
        CHECK(inst.state("CLOSE") == "Created");
    }

    TEST_CASE("Mediator: completion path runs the reroute branch and skips hold") {
        ParticipantDirectory directory;
        addLogisticsStaff(directory);
        std::unique_ptr<SubProcess> definition(buildShipmentExceptionTree());
        TestInstance inst(*definition, shipmentExceptionRules(), directory);
        REQUIRE(inst.root->makeAvailable());

        REQUIRE(finishWith(inst.item("LOG_EXCEPTION"), "logged"));
        CHECK(inst.state("CLASSIFY") == "Assigned");

        REQUIRE(finishWith(inst.item("CLASSIFY"), "high"));
        CHECK(inst.state("REPORT") == "Completed");
        CHECK(inst.state("HANDLE") == "InProgress");
        CHECK(inst.state("ASSESS") == "Assigned");
        CHECK(inst.state("REROUTE") == "Created");

        REQUIRE(finishWith(inst.item("ASSESS"), "reroute"));
        CHECK(inst.state("REROUTE") == "Assigned");
        CHECK(inst.item("REROUTE")->getAssignee()->getName() == "Lerato Mokoena");
        CHECK(inst.state("HOLD") == "Cancelled");     // branch not taken
        CHECK(inst.state("HANDLE") == "InProgress");

        REQUIRE(finishWith(inst.item("REROUTE"), "rerouted"));
        CHECK(inst.state("HANDLE") == "Completed");
        CHECK(inst.state("NOTIFY_CUSTOMER") == "Assigned");

        REQUIRE(finishWith(inst.item("NOTIFY_CUSTOMER"), "sent"));
        REQUIRE(finishWith(inst.item("CLOSE_CASE"), "closed"));
        CHECK(inst.state("CLOSE") == "Completed");
        CHECK(inst.state("SHIP_EX") == "Completed");
    }

    TEST_CASE("Mediator: the other outcome opens hold and skips reroute") {
        ParticipantDirectory directory;
        addLogisticsStaff(directory);
        std::unique_ptr<SubProcess> definition(buildShipmentExceptionTree());
        TestInstance inst(*definition, shipmentExceptionRules(), directory);
        REQUIRE(inst.root->makeAvailable());
        REQUIRE(finishWith(inst.item("LOG_EXCEPTION"), "logged"));
        REQUIRE(finishWith(inst.item("CLASSIFY"), "low"));

        REQUIRE(finishWith(inst.item("ASSESS"), "hold"));
        CHECK(inst.state("HOLD") == "Assigned");
        CHECK(inst.item("HOLD")->getAssignee()->getName() == "Pieter Botha");
        CHECK(inst.state("REROUTE") == "Cancelled");
    }

    TEST_CASE("Mediator: parallel entry items join before the next item opens") {
        ParticipantDirectory directory;
        addBursaryApprovers(directory);
        directory.addParticipant(new Participant("U1", "Kabelo Moloi", "AdmissionsOfficer"));
        directory.addParticipant(new Participant("U2", "Dr. Pillay", "FacultyReviewer"));

        SubProcess definition("REVIEW", "Application Review");
        definition.add(roleTask("VERIFY_DOCS", "Verify Documents", "AdmissionsOfficer"));
        definition.add(roleTask("ACADEMIC", "Academic Review", "FacultyReviewer"));
        definition.add(roleTask("DECIDE", "Final Decision", "BursaryCommittee"));
        std::vector<DependencyRule> rules = {
            DependencyRule("VERIFY_DOCS", "DECIDE", "*"),
            DependencyRule("ACADEMIC", "DECIDE", "*")};
        TestInstance inst(definition, rules, directory);

        REQUIRE(inst.root->makeAvailable());
        CHECK(inst.state("VERIFY_DOCS") == "Assigned");  // both entry items run in parallel
        CHECK(inst.state("ACADEMIC") == "Assigned");
        CHECK(inst.state("DECIDE") == "Created");

        REQUIRE(finishWith(inst.item("ACADEMIC"), "meets-requirements"));
        CHECK(inst.state("DECIDE") == "Created");        // still waiting on VERIFY_DOCS

        REQUIRE(finishWith(inst.item("VERIFY_DOCS"), "verified"));
        CHECK(inst.state("DECIDE") == "Assigned");
        CHECK(inst.state("REVIEW") == "InProgress");
    }

    TEST_CASE("Mediator: a join after an either/or branch opens once the taken branch finishes") {
        ParticipantDirectory directory;
        addLogisticsStaff(directory);

        SubProcess definition("HANDLE", "Exception Handling");
        definition.add(roleTask("ASSESS", "Assess Impact", "LogisticsCoordinator"));
        definition.add(roleTask("REROUTE", "Reroute Shipment", "Dispatcher"));
        definition.add(roleTask("HOLD", "Hold Shipment", "WarehouseSupervisor"));
        definition.add(roleTask("NOTIFY_CUSTOMER", "Notify Customer", "CustomerService"));
        std::vector<DependencyRule> rules = {
            DependencyRule("ASSESS", "REROUTE", "reroute"),
            DependencyRule("ASSESS", "HOLD", "hold"),
            DependencyRule("REROUTE", "NOTIFY_CUSTOMER", "*"),
            DependencyRule("HOLD", "NOTIFY_CUSTOMER", "*")};
        TestInstance inst(definition, rules, directory);

        REQUIRE(inst.root->makeAvailable());
        REQUIRE(finishWith(inst.item("ASSESS"), "hold"));
        CHECK(inst.state("REROUTE") == "Cancelled");
        CHECK(inst.state("NOTIFY_CUSTOMER") == "Created"); // HOLD still running

        REQUIRE(finishWith(inst.item("HOLD"), "held"));
        CHECK(inst.state("NOTIFY_CUSTOMER") == "Assigned");
    }

    TEST_CASE("Mediator: an unhandled rejection fails the branch and cancels the rest") {
        ParticipantDirectory directory;
        addLogisticsStaff(directory);
        std::unique_ptr<SubProcess> definition(buildShipmentExceptionTree());
        TestInstance inst(*definition, shipmentExceptionRules(), directory);
        REQUIRE(inst.root->makeAvailable());

        WorkItem* log = inst.item("LOG_EXCEPTION");
        REQUIRE(log->start());
        REQUIRE(log->reject("waybill not found"));

        CHECK(inst.state("CLASSIFY") == "Cancelled");
        CHECK(inst.state("REPORT") == "Rejected");
        CHECK(inst.item("REPORT")->getOutcome() == "Log Exception rejected: waybill not found");
        CHECK(inst.state("SHIP_EX") == "Rejected");
        CHECK(inst.state("HANDLE") == "Cancelled");
        CHECK(inst.state("ASSESS") == "Cancelled");
        CHECK(inst.state("CLOSE") == "Cancelled");
    }

    TEST_CASE("Mediator: a 'rejected' rule recovers from rejection and skips the normal path") {
        ParticipantDirectory directory;
        addBursaryApprovers(directory);
        directory.addParticipant(new Participant("U1", "Kabelo Moloi", "AdmissionsOfficer"));

        SubProcess definition("FUNDING", "Bursary Funding");
        definition.add(roleTask("APPROVE", "Approve Bursary", "FinanceManager"));
        definition.add(roleTask("PAY", "Pay Bursary", "BursaryOfficer"));
        definition.add(roleTask("APPEAL", "Notify Appeal Options", "AdmissionsOfficer"));
        std::vector<DependencyRule> rules = {
            DependencyRule("APPROVE", "PAY", "approved"),
            DependencyRule("APPROVE", "APPEAL", "rejected")};
        TestInstance inst(definition, rules, directory);

        REQUIRE(inst.root->makeAvailable());
        WorkItem* approve = inst.item("APPROVE");
        REQUIRE(approve->start());
        REQUIRE(approve->reject("income above threshold"));

        CHECK(inst.state("APPEAL") == "Assigned");
        CHECK(inst.state("PAY") == "Cancelled");
        CHECK(inst.state("FUNDING") == "InProgress");  // the branch is not failed

        REQUIRE(finishWith(inst.item("APPEAL"), "sent"));
        CHECK(inst.state("FUNDING") == "Completed");
    }

    TEST_CASE("Mediator: invalid actions are refused and do not move work forward") {
        ParticipantDirectory directory;
        addLogisticsStaff(directory);
        std::unique_ptr<SubProcess> definition(buildShipmentExceptionTree());
        TestInstance inst(*definition, shipmentExceptionRules(), directory);
        REQUIRE(inst.root->makeAvailable());

        // CLASSIFY has not been opened yet, so it cannot be completed.
        CHECK_FALSE(inst.item("CLASSIFY")->complete("high"));
        // LOG_EXCEPTION is assigned but not started.
        CHECK_FALSE(inst.item("LOG_EXCEPTION")->complete("logged"));
        CHECK(inst.state("CLASSIFY") == "Created");
        CHECK(inst.state("HANDLE") == "Created");
        // Starting an already running sub-process again is refused.
        CHECK_FALSE(inst.root->makeAvailable());
    }

    TEST_CASE("Mediator: two instances of one definition progress independently") {
        ParticipantDirectory directory;
        addLogisticsStaff(directory);
        std::unique_ptr<SubProcess> definition(buildShipmentExceptionTree());
        TestInstance first(*definition, shipmentExceptionRules(), directory);
        TestInstance second(*definition, shipmentExceptionRules(), directory);

        REQUIRE(first.root->makeAvailable());
        REQUIRE(finishWith(first.item("LOG_EXCEPTION"), "logged"));
        REQUIRE(first.item("CLASSIFY")->start());
        REQUIRE(first.item("CLASSIFY")->reject("duplicate report"));

        CHECK(first.state("SHIP_EX") == "Rejected");
        CHECK(second.state("SHIP_EX") == "Created");
        CHECK(second.state("LOG_EXCEPTION") == "Created");
        CHECK(definition->find("SHIP_EX")->getStateName() == "Created");

        REQUIRE(second.root->makeAvailable());
        CHECK(second.state("LOG_EXCEPTION") == "Assigned");
    }

    TEST_CASE("Mediator: an empty sub-process completes as soon as it starts") {
        SubProcess definition("ROOT", "Root");
        definition.add(new SubProcess("EMPTY", "Nothing To Do"));
        ParticipantDirectory directory;
        TestInstance inst(definition, {}, directory);

        REQUIRE(inst.root->makeAvailable());
        CHECK(inst.state("EMPTY") == "Completed");
        CHECK(inst.state("ROOT") == "Completed");
    }
}

