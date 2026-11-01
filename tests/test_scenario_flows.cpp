#include "doctest.h"
#include "TividyWorkflowFacade.h"
#include "WorkflowInstance.h"
#include "WorkItem.h"
#include "Participant.h"
#include "WorkflowEventListener.h"
#include "GovernmentIDAdapter.h"
#include "BankGatewayAdapter.h"
#include "TrackingServiceAdapter.h"
#include "CarrierAdapter.h"

#include <string>
#include <vector>

namespace {
class ScenarioEvents : public WorkflowEventListener {
public:
    std::vector<WorkflowEvent> events;
    void onEvent(const WorkflowEvent& event) override { events.push_back(event); }
};

struct ScenarioFixture {
    TividyWorkflowFacade facade;
    ScenarioEvents* events;
    ScenarioFixture() : events(new ScenarioEvents()) {
        const char* roles[] = {"AdmissionsOfficer", "FacultyReviewer", "BursaryCommittee",
            "BursaryOfficer", "FinanceManager", "WarehouseClerk", "ExceptionAnalyst",
            "LogisticsCoordinator", "Dispatcher", "WarehouseSupervisor", "CustomerService"};
        for (const char* role : roles) facade.addParticipant(role, role, {role});
        facade.addService(new GovernmentIDAdapter());
        facade.addService(new BankGatewayAdapter());
        facade.addService(new TrackingServiceAdapter());
        facade.addService(new CarrierAdapter());
        facade.addListener(events);
    }

    WorkflowInstance* launch(const std::string& path, const InstanceData& data) {
        const std::string definition = facade.registerDefinitionFromFile(path);
        REQUIRE_FALSE(definition.empty());
        const std::string id = facade.startInstance(definition, data);
        REQUIRE_FALSE(id.empty());
        WorkflowInstance* instance = facade.getInstance(id);
        REQUIRE(instance != nullptr);
        return instance;
    }

    void finish(WorkflowInstance* instance, const std::string& itemId,
                const std::string& outcome = "done") {
        INFO("Complete ", itemId);
        WorkItem* item = instance->findItem(itemId);
        REQUIRE(item != nullptr);
        REQUIRE(item->getStateName() == "Assigned");
        Participant* actor = item->getAssignee();
        REQUIRE(actor != nullptr);
        REQUIRE(facade.start(instance->getId(), itemId, actor));
        REQUIRE(facade.complete(instance->getId(), itemId, actor, outcome));
        CHECK(item->getStateName() == "Completed");
    }

    void reject(WorkflowInstance* instance, const std::string& itemId) {
        WorkItem* item = instance->findItem(itemId);
        REQUIRE(item != nullptr);
        Participant* actor = item->getAssignee();
        REQUIRE(actor != nullptr);
        REQUIRE(facade.start(instance->getId(), itemId, actor));
        REQUIRE(facade.reject(instance->getId(), itemId, actor, "Documents are incomplete"));
    }

    void reachFunding(WorkflowInstance* instance) {
        finish(instance, "CAPTURE");
        CHECK(instance->findItem("VERIFY_DOCS")->getStateName() == "Created");
        finish(instance, "VERIFY_ID");
        CHECK(instance->getData().get("identityVerified") == "true");
        CHECK(instance->findItem("VERIFY_DOCS")->getStateName() == "Assigned");
        CHECK(instance->findItem("ACADEMIC")->getStateName() == "Assigned");
        finish(instance, "ACADEMIC");
        CHECK(instance->findItem("DECIDE")->getStateName() == "Created");
        finish(instance, "VERIFY_DOCS");
        finish(instance, "DECIDE", "approved");
    }
};

InstanceData bursaryData() {
    InstanceData data;
    data.set("idNumber", "0501015800087");
    data.set("surname", "Mokoena");
    data.set("incomeBracket", "low");
    data.set("amount", "15000.50");
    data.set("accountNumber", "62012345678");
    data.set("branchCode", "250655");
    data.set("reference", "TEST-BURSARY");
    return data;
}

InstanceData shipmentData() {
    InstanceData data;
    data.set("waybill", "WB1002");
    data.set("destinationHub", "Cape Town");
    return data;
}
}

TEST_SUITE("Prepared workflows through the public facade") {
    TEST_CASE("University successful funding uses routed decorated tasks and simulated services") {
        ScenarioFixture fixture;
        WorkflowInstance* instance = fixture.launch("definitions/university_bursary.txt", bursaryData());
        CHECK(instance->findItem("VERIFY_ID")->getStateName() == "Created");
        fixture.reachFunding(instance);
        fixture.finish(instance, "FUNDING_DOCS");
        CHECK(instance->findItem("APPROVE")->getAssignee()->hasRole("BursaryOfficer"));
        fixture.finish(instance, "APPROVE", "approved");
        CHECK(instance->findItem("APPEAL")->getStateName() == "Cancelled");
        fixture.finish(instance, "PAY");
        CHECK(instance->getData().get("paymentStatus") == "PAID");
        CHECK(instance->getData().get("paymentReference").find("EFT") == 0);
        CHECK(fixture.facade.getStatus(instance->getId()) == "completed");
        REQUIRE_FALSE(fixture.events->events.empty());
        for (const WorkflowEvent& event : fixture.events->events) {
            CHECK(event.instanceId == instance->getId());
        }
    }

    TEST_CASE("University funding rejection opens appeal and never payment") {
        ScenarioFixture fixture;
        WorkflowInstance* instance = fixture.launch("definitions/university_bursary.txt", bursaryData());
        fixture.reachFunding(instance);
        SUBCASE("Funding documents are rejected") { fixture.reject(instance, "FUNDING_DOCS"); }
        SUBCASE("Bursary approval is rejected") {
            fixture.finish(instance, "FUNDING_DOCS");
            fixture.reject(instance, "APPROVE");
        }
        CHECK(instance->findItem("PAY")->getStateName() == "Cancelled");
        fixture.finish(instance, "APPEAL");
        CHECK(instance->getData().get("paymentReference").empty());
        CHECK(fixture.facade.getStatus(instance->getId()) == "completed");
    }

    TEST_CASE("University negative identity result rejects the application") {
        ScenarioFixture fixture;
        InstanceData data = bursaryData();
        data.set("idNumber", "9999999999999");
        WorkflowInstance* instance = fixture.launch("definitions/university_bursary.txt", data);
        fixture.finish(instance, "CAPTURE");
        WorkItem* item = instance->findItem("VERIFY_ID");
        Participant* actor = item->getAssignee();
        REQUIRE(actor != nullptr);
        REQUIRE(fixture.facade.start(instance->getId(), "VERIFY_ID", actor));
        REQUIRE(fixture.facade.complete(instance->getId(), "VERIFY_ID", actor, "verified"));
        CHECK(item->getStateName() == "Rejected");
        CHECK(instance->getData().get("identityVerified") == "false");
        CHECK(instance->findItem("REVIEW")->getStateName() == "Cancelled");
        CHECK(instance->findItem("FUNDING")->getStateName() == "Cancelled");
        CHECK(fixture.facade.getStatus(instance->getId()) == "terminated");
    }

    TEST_CASE("Shipment decisions run either hold or reroute and then close") {
        ScenarioFixture fixture;
        WorkflowInstance* instance = fixture.launch("definitions/shipment_exception.txt", shipmentData());
        fixture.finish(instance, "LOG_EXCEPTION");
        fixture.finish(instance, "CLASSIFY");
        CHECK(instance->getData().get("shipmentStatus") == "DELAYED");
        CHECK(instance->getData().get("delayHours") == "9");
        CHECK(instance->findItem("CLOSE")->getStateName() == "Created");
        SUBCASE("Reroute uses carrier adapter") {
            fixture.finish(instance, "ASSESS", "reroute");
            CHECK(instance->findItem("HOLD")->getStateName() == "Cancelled");
            fixture.finish(instance, "REROUTE");
            CHECK(instance->getData().get("newHub") == "Cape Town");
            CHECK_FALSE(instance->getData().get("rerouteReference").empty());
        }
        SUBCASE("Hold skips carrier call") {
            fixture.finish(instance, "ASSESS", "hold");
            CHECK(instance->findItem("REROUTE")->getStateName() == "Cancelled");
            fixture.finish(instance, "HOLD");
            CHECK(instance->getData().get("rerouteReference").empty());
        }
        CHECK(instance->findItem("CLOSE_CASE")->getStateName() == "Created");
        fixture.finish(instance, "NOTIFY_CUSTOMER");
        fixture.finish(instance, "CLOSE_CASE");
        CHECK(fixture.facade.getStatus(instance->getId()) == "completed");
    }

    TEST_CASE("Shipment unrecovered rejection terminates unopened branches") {
        ScenarioFixture fixture;
        WorkflowInstance* instance = fixture.launch("definitions/shipment_exception.txt", shipmentData());
        fixture.reject(instance, "LOG_EXCEPTION");
        CHECK(fixture.facade.getStatus(instance->getId()) == "terminated");
        CHECK(instance->findItem("CLASSIFY")->getStateName() == "Cancelled");
        CHECK(instance->findItem("HANDLE")->getStateName() == "Cancelled");
        CHECK(instance->findItem("CLOSE")->getStateName() == "Cancelled");
    }

    TEST_CASE("Two live prepared instances keep independent data state and undo") {
        ScenarioFixture fixture;
        WorkflowInstance* first = fixture.launch("definitions/shipment_exception.txt", shipmentData());
        InstanceData secondData = shipmentData();
        secondData.set("waybill", "WB1003");
        const std::string secondId = fixture.facade.startInstance("SHIP_EX", secondData);
        WorkflowInstance* second = fixture.facade.getInstance(secondId);
        REQUIRE(second != nullptr);
        fixture.finish(first, "LOG_EXCEPTION");
        fixture.finish(first, "CLASSIFY");
        CHECK(second->findItem("LOG_EXCEPTION")->getStateName() == "Assigned");
        CHECK(second->findItem("CLASSIFY")->getStateName() == "Created");
        CHECK(second->getData().get("shipmentStatus").empty());
        REQUIRE(fixture.facade.undoLast());
        CHECK(first->findItem("CLASSIFY")->getStateName() == "InProgress");
        CHECK(first->findItem("HANDLE")->getStateName() == "Created");
        CHECK(first->getData().get("shipmentStatus").empty());
        WorkItem* classify = first->findItem("CLASSIFY");
        REQUIRE(fixture.facade.complete(first->getId(), "CLASSIFY", classify->getAssignee(), "tracked"));
        CHECK(first->findItem("ASSESS")->getStateName() == "Assigned");
        CHECK(second->findItem("ASSESS")->getStateName() == "Created");
    }
}
