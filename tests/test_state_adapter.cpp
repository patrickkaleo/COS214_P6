/**
 * @file test_state_adapter.cpp
 * @brief Unit tests for State & Adapter subsystems.
 * @author Angela Ramaboea
 */

#include "doctest.h"

#include <memory>
#include <vector>
#include <string>
#include <map>

#include "WorkItem.h"
#include "WorkItemState.h"
#include "WorkflowEvent.h"
#include "WorkflowEventListener.h"
#include "EventDispatcher.h"
#include "Participant.h"

#include "ExternalService.h"
#include "ServiceRegistry.h"
#include "GovernmentIDAdapter.h"
#include "BankGatewayAdapter.h"
#include "TrackingServiceAdapter.h"
#include "CarrierAdapter.h"
#include "InstanceData.h"

// ---------------------------------------------------------------------------
// State pattern test helpers
// ---------------------------------------------------------------------------

class LifecycleTestItem : public WorkItem {
public:
    int availableHookCalls;

    LifecycleTestItem(const std::string& id, const std::string& itemName)
        : WorkItem(id, itemName), availableHookCalls(0) {}

    WorkItem* clone() const override { return new LifecycleTestItem(*this); }

protected:
    void onAvailable() override { ++availableHookCalls; }
};

class EventRecorder : public WorkflowEventListener {
public:
    std::vector<WorkflowEvent> events;
    void onEvent(const WorkflowEvent& e) override { events.push_back(e); }
};

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

    TEST_CASE("State: cancellation marks the item finished") {
        LifecycleTestItem item("T3", "Fee Verification");
        CHECK(item.makeAvailable());
        CHECK(item.cancel());
        CHECK(item.getStateName() == "Cancelled");
        CHECK(item.getOutcome().empty());
        CHECK(item.isFinished());
    }

    TEST_CASE("State: escalation bumps the level and assigns the escalation participant") {
        Participant clerk("P3", "Sipho", "Clerk");
        Participant supervisor("P4", "Nomvula", "Supervisor");
        LifecycleTestItem item("T4", "Payment Gateway Timeout");

        driveToInProgress(item, &clerk);
        CHECK(item.getEscalationLevel() == 0);

        CHECK(item.escalate());
        CHECK(item.getStateName() == "Escalated");
        CHECK(item.getEscalationLevel() == 1);
        CHECK(item.getAssignee() == nullptr);

        // Escalated work must be re-assigned before it can continue
        CHECK_FALSE(item.start());
        CHECK_FALSE(item.complete("approved"));
        CHECK(item.getStateName() == "Escalated");

        CHECK(item.assign(&supervisor));
        CHECK(item.getStateName() == "Assigned");
        CHECK(item.getAssignee() == &supervisor);

        CHECK(item.start());
        CHECK(item.escalate());
        CHECK(item.getEscalationLevel() == 2);
    }

    TEST_CASE("State: illegal transitions are rejected and leave the state unchanged") {
        LifecycleTestItem item("T5", "Direct Start Attempt");
        Participant p("P5", "Kagiso", "Clerk");

        CHECK_FALSE(item.start());
        CHECK(item.getStateName() == "Created");
        CHECK_FALSE(item.complete("done"));
        CHECK(item.getStateName() == "Created");

        CHECK(item.makeAvailable());
        CHECK(item.assign(&p));
        CHECK(item.start());
        CHECK(item.complete("done"));
        CHECK(item.getStateName() == "Completed");

        CHECK_FALSE(item.makeAvailable());
        CHECK_FALSE(item.assign(&p));
        CHECK_FALSE(item.start());
        CHECK_FALSE(item.complete("again"));
        CHECK_FALSE(item.reject("late reject"));
        CHECK_FALSE(item.cancel());
        CHECK_FALSE(item.escalate());
        CHECK(item.getStateName() == "Completed");
    }

    TEST_CASE("State: copy constructor performs a deep copy of the current state") {
        Participant clerk("P6", "Lerato", "Clerk");
        LifecycleTestItem original("T6", "Original");
        original.makeAvailable();
        original.assign(&clerk);

        LifecycleTestItem copy(original);
        CHECK(copy.getStateName() == "Assigned");
        CHECK(copy.getAssignee() == &clerk);

        CHECK(copy.start());
        CHECK(copy.getStateName() == "InProgress");
        CHECK(original.getStateName() == "Assigned");

        std::unique_ptr<WorkItem> cloned(original.clone());
        CHECK(cloned->getStateName() == "Assigned");
        CHECK(cloned->cancel());
        CHECK(original.getStateName() == "Assigned");
    }

    TEST_CASE("State: every transition publishes the matching event on the dispatcher") {
        EventDispatcher& dispatcher = EventDispatcher::getInstance();
        dispatcher.clearListeners();

        auto recorder = std::make_shared<EventRecorder>();
        dispatcher.attachListener(recorder);

        Participant clerk("P7", "Bongani", "Clerk");
        LifecycleTestItem item("T7", "Event Tracing Item");
        item.setInstanceId("INST-42");

        item.makeAvailable();
        item.assign(&clerk);
        item.start();
        item.escalate();
        item.assign(&clerk);
        item.start();
        item.reject("failed review");

        REQUIRE(recorder->events.size() == 7);
        CHECK(recorder->events[0].type == EventType::AVAILABLE);
        CHECK(recorder->events[1].type == EventType::ASSIGNED);
        CHECK(recorder->events[2].type == EventType::STARTED);
        CHECK(recorder->events[3].type == EventType::ESCALATED);
        CHECK(recorder->events[4].type == EventType::ASSIGNED);
        CHECK(recorder->events[5].type == EventType::STARTED);
        CHECK(recorder->events[6].type == EventType::REJECTED);

        for (const auto& ev : recorder->events) {
            CHECK(ev.instanceId == "INST-42");
            CHECK(ev.itemId == "T7");
        }

        dispatcher.clearListeners();
    }

    TEST_CASE("State: requests are refused in Created, Available and Assigned until legal") {
        Participant clerk("P8", "Sipho", "WarehouseClerk");
        LifecycleTestItem item("T8", "Log Exception");

        // Created: cannot be assigned, rejected or escalated
        CHECK_FALSE(item.assign(&clerk));
        CHECK_FALSE(item.reject("no"));
        CHECK_FALSE(item.escalate());
        CHECK(item.getStateName() == "Created");

        // Available: cannot be made available again, started or completed
        REQUIRE(item.makeAvailable());
        CHECK_FALSE(item.makeAvailable());
        CHECK_FALSE(item.start());
        CHECK_FALSE(item.complete("done"));
        CHECK(item.getStateName() == "Available");

        // Assigned: cannot complete or reject without starting
        REQUIRE(item.assign(&clerk));
        CHECK_FALSE(item.complete("done"));
        CHECK_FALSE(item.reject("no"));
        CHECK(item.getStateName() == "Assigned");
    }

    TEST_CASE("State: assigning a null participant is refused") {
        LifecycleTestItem item("T9", "Classify Severity");
        REQUIRE(item.makeAvailable());
        CHECK_FALSE(item.assign(nullptr));
        CHECK(item.getStateName() == "Available");
        CHECK(item.getAssignee() == nullptr);

        Participant clerk("P9", "Thabo", "Clerk");
        REQUIRE(item.assign(&clerk));
        CHECK_FALSE(item.assign(nullptr));
        CHECK(item.getStateName() == "Assigned");
        CHECK(item.getAssignee() == &clerk);

        REQUIRE(item.escalate());
        CHECK_FALSE(item.assign(nullptr));
        CHECK(item.getStateName() == "Escalated");
    }

    TEST_CASE("State: delegation re-assigns an Assigned item to another participant") {
        Participant a("P10", "Kabelo", "Registrar");
        Participant b("P11", "Zanele", "Registrar");
        LifecycleTestItem item("T10", "Verify Documents");
        REQUIRE(item.makeAvailable());
        REQUIRE(item.assign(&a));

        CHECK(item.assign(&b));
        CHECK(item.getStateName() == "Assigned");
        CHECK(item.getAssignee() == &b);
    }

    TEST_CASE("State: cancellation is allowed from every non-final state") {
        Participant p("P12", "Ayanda", "CustomerService");

        LifecycleTestItem created("C1", "Created item");
        CHECK(created.cancel());
        CHECK(created.getStateName() == "Cancelled");
        CHECK(created.isFinished());

        LifecycleTestItem available("C2", "Available item");
        REQUIRE(available.makeAvailable());
        CHECK(available.cancel());
        CHECK(available.getStateName() == "Cancelled");

        LifecycleTestItem assigned("C3", "Assigned item");
        REQUIRE(assigned.makeAvailable());
        REQUIRE(assigned.assign(&p));
        CHECK(assigned.cancel());
        CHECK(assigned.getStateName() == "Cancelled");

        LifecycleTestItem inProgress("C4", "In-progress item");
        driveToInProgress(inProgress, &p);
        CHECK(inProgress.cancel());
        CHECK(inProgress.getStateName() == "Cancelled");

        LifecycleTestItem escalated("C5", "Escalated item");
        REQUIRE(escalated.makeAvailable());
        REQUIRE(escalated.escalate());
        CHECK(escalated.cancel());
        CHECK(escalated.getStateName() == "Cancelled");
    }

    TEST_CASE("State: refused requests publish no event") {
        EventDispatcher& dispatcher = EventDispatcher::getInstance();
        dispatcher.clearListeners();

        auto recorder = std::make_shared<EventRecorder>();
        dispatcher.attachListener(recorder);

        Participant p("P13", "Musa", "Dean");
        LifecycleTestItem item("T11", "Admission Decision");

        CHECK_FALSE(item.start());
        CHECK(recorder->events.empty());

        driveToInProgress(item, &p);
        REQUIRE(item.complete("approved"));
        REQUIRE(recorder->events.size() == 4);
        CHECK(recorder->events[3].outcome == "approved");

        CHECK_FALSE(item.start());
        CHECK(recorder->events.size() == 4);

        dispatcher.clearListeners();
    }
}

// ---------------------------------------------------------------------------
// Adapter pattern tests
// ---------------------------------------------------------------------------

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
        CHECK(registry.has("IdentityVerificationService"));
        CHECK(registry.has("PaymentGateway"));
        CHECK(registry.has("TrackingService"));
        CHECK(registry.has("CarrierAdapter"));
        CHECK(registry.find("NoSuchService") == nullptr);

        ExternalService* svc = registry.find("TrackingService");
        REQUIRE(svc != nullptr);
        CHECK(svc->name() == "TrackingService");
    }

    TEST_CASE("Adapter: registering a service under an existing name replaces it") {
        ServiceRegistry registry;
        GovernmentIDAdapter* first = new GovernmentIDAdapter();
        GovernmentIDAdapter* second = new GovernmentIDAdapter();

        registry.add(first);
        CHECK(registry.find("IdentityVerificationService") == first);

        registry.add(second);
        CHECK(registry.find("IdentityVerificationService") == second);
        CHECK(registry.size() == 1);
    }

    TEST_CASE("Adapter: registry ignores null adapters and owns the rest") {
        ServiceRegistry registry;
        registry.add(new GovernmentIDAdapter());
        registry.add(new BankGatewayAdapter());
        registry.add(nullptr);

        CHECK(registry.size() == 2); // the destructor deletes both; make valgrind checks this
    }

    TEST_CASE("Adapter: GovernmentIDAdapter verifies a matching identity") {
        GovernmentIDAdapter adapter;
        ServiceResult r = adapter.invoke(makeRequest("verifyIdentity",
            {{"idNumber", "0501015800087"}, {"surname", "Mokoena"}}));

        CHECK(r.success);
        CHECK(r.outputs["identityVerified"] == "true");
        CHECK(r.outputs["identityStatus"] == "VERIFIED");
    }

    TEST_CASE("Adapter: GovernmentIDAdapter reports unknown IDs as data and fails on missing inputs") {
        GovernmentIDAdapter adapter;

        // A negative answer is a successful call: the workflow branches on it.
        ServiceResult unknown = adapter.invoke(makeRequest("verifyIdentity",
            {{"idNumber", "9999999999999"}, {"surname", "Nobody"}}));
        CHECK(unknown.success);
        CHECK(unknown.outputs["identityVerified"] == "false");
        CHECK(unknown.outputs["identityStatus"] == "NOT_FOUND");

        ServiceResult missing = adapter.invoke(makeRequest("verifyIdentity", {}));
        CHECK_FALSE(missing.success);

        CHECK_FALSE(adapter.invoke(makeRequest("verifyIdentity",
            {{"idNumber", "12AB"}, {"surname", "Mokoena"}})).success);
    }

    TEST_CASE("Adapter: GovernmentIDAdapter reports a mismatch or deceased record as data") {
        GovernmentIDAdapter adapter;

        ServiceResult mismatch = adapter.invoke(makeRequest("verifyIdentity",
            {{"idNumber", "0501015800087"}, {"surname", "Naidoo"}}));
        CHECK(mismatch.success);
        CHECK(mismatch.outputs["identityVerified"] == "false");
        CHECK(mismatch.outputs["identityStatus"] == "SURNAME_MISMATCH");

        ServiceResult deceased = adapter.invoke(makeRequest("verifyIdentity",
            {{"idNumber", "4808105009083"}, {"surname", "Dlamini"}}));
        CHECK(deceased.success);
        CHECK(deceased.outputs["identityVerified"] == "false");
        CHECK(deceased.outputs["identityStatus"] == "DECEASED");
    }

    TEST_CASE("Adapter: GovernmentIDAdapter handles third-party service outages") {
        GovernmentIDAdapter adapter;
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
        BankGatewayAdapter adapter(new BankPaymentGateway(5000000LL));

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
