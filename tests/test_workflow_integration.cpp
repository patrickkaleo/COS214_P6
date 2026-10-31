/**
 * @file test_workflow_integration.cpp
 * @brief End-to-end tests running both industries' definitions through the facade.
 * @author Angela Ramaboea
 */

#include "doctest.h"

#include "BankGatewayAdapter.h"
#include "CarrierAdapter.h"
#include "GovernmentIDAdapter.h"
#include "Participant.h"
#include "TividyWorkflowFacade.h"
#include "TrackingServiceAdapter.h"

#include <ctime>
#include <map>
#include <string>
#include <vector>

namespace {

const char* const EDUCATION = "data/education_admissions.workflow";
const char* const LOGISTICS = "data/logistics_shipment_exception.workflow";

/// A facade loaded with both industries, as the TUI sets it up.
class Fixture {
public:
    TividyWorkflowFacade facade;
    std::string education;
    std::string logistics;

    Fixture() {
        facade.addService(new GovernmentIDAdapter(nullptr));
        facade.addService(new BankGatewayAdapter(nullptr));
        facade.addService(new TrackingServiceAdapter(nullptr));
        facade.addService(new CarrierAdapter(nullptr));

        education = facade.registerDefinitionFromFile(EDUCATION);
        logistics = facade.registerDefinitionFromFile(LOGISTICS);

        facade.addParticipant("clerk", "Clerk", {"Admissions Clerk"});
        facade.addParticipant("officer", "Officer", {"Admissions Officer"});
        facade.addParticipant("eng", "Engineer", {"Engineering Reviewer"});
        facade.addParticipant("aid", "Aid", {"Financial Aid Officer"});
        facade.addParticipant("mgr", "Manager", {"Faculty Manager"});
        facade.addParticipant("dean", "Dean", {"Dean"});
        facade.addParticipant("fin", "Finance", {"Finance Clerk"});
        facade.addParticipant("wh", "Warehouse", {"Warehouse Clerk"});
        facade.addParticipant("sup", "Supervisor", {"Warehouse Supervisor"});
        facade.addParticipant("analyst", "Analyst", {"Exception Analyst"});
        facade.addParticipant("liaison", "Liaison", {"Customer Liaison"});
    }

    WorkItemInfo item(const std::string& instance, const std::string& id) {
        for (const WorkItemInfo& info : facade.getWorkItems(instance)) {
            if (info.id == id) {
                return info;
            }
        }
        FAIL("no work item " << id);
        return WorkItemInfo();
    }

    std::string state(const std::string& instance, const std::string& id) {
        return item(instance, id).state;
    }

    Participant* who(const std::string& id) {
        return facade.findParticipant(id);
    }

    /// Starts and completes an item as whoever it was routed to.
    void finish(const std::string& instance, const std::string& id, const std::string& outcome) {
        Participant* assignee = who(item(instance, id).assigneeId);
        REQUIRE(assignee != nullptr);
        REQUIRE(facade.start(instance, id, assignee));
        REQUIRE(facade.complete(instance, id, assignee, outcome));
    }
};

InstanceData educationData(const std::string& amount = "45000") {
    InstanceData data;
    data.set("surname", "Mokoena");
    data.set("idNumber", "0501015800087");
    data.set("faculty", "Engineering");
    data.set("amount", amount);
    data.set("accountNumber", "123456789");
    data.set("branchCode", "250655");
    data.set("reference", "REF-1");
    return data;
}

InstanceData logisticsData(const std::string& severity = "high", const std::string& hub = "Cape Town") {
    InstanceData data;
    data.set("waybill", "WB1002");
    data.set("severity", severity);
    data.set("destinationHub", hub);
    return data;
}

} // namespace

TEST_SUITE("Workflow integration through the facade") {

    TEST_CASE("Starting an instance opens only entry work and routes it") {
        Fixture f;
        const std::string id = f.facade.startInstance(f.education, educationData());
        REQUIRE_FALSE(id.empty());

        CHECK(f.facade.getStatus(id) == "running");
        CHECK(f.state(id, "CAPTURE") == "Assigned");
        CHECK(f.item(id, "CAPTURE").assigneeId == "clerk");
        CHECK(f.state(id, "VERIFY_ID") == "Created");
        CHECK(f.state(id, "REVIEW") == "Created");
    }

    TEST_CASE("Education: admit, bursary, payment and close completes the instance") {
        Fixture f;
        const std::string id = f.facade.startInstance(f.education, educationData());

        f.finish(id, "CAPTURE", "captured");
        CHECK(f.state(id, "VERIFY_ID") == "Assigned");

        // Starting the task calls the identity adapter with the instance's data
        REQUIRE(f.facade.start(id, "VERIFY_ID", f.who("clerk")));
        CHECK(f.state(id, "VERIFY_ID") == "InProgress");
        CHECK(f.facade.getInstanceData(id)["identityVerified"] == "true");
        REQUIRE(f.facade.complete(id, "VERIFY_ID", f.who("clerk"), "verified"));

        // The finished sub-process opens the next step, routed on the faculty field
        CHECK(f.state(id, "INTAKE") == "Completed");
        CHECK(f.item(id, "REVIEW").assigneeId == "eng");

        f.finish(id, "REVIEW", "admit");
        CHECK(f.state(id, "REGRET") == "Cancelled");

        f.finish(id, "OFFER", "bursary requested");
        CHECK(f.item(id, "BURSARY").assigneeId == "aid");

        f.finish(id, "BURSARY", "approved");
        REQUIRE(f.facade.start(id, "PAY", f.who("fin")));
        CHECK(f.facade.getInstanceData(id)["paymentStatus"] == "PAID");
        REQUIRE(f.facade.complete(id, "PAY", f.who("fin"), "paid"));

        CHECK(f.facade.getStatus(id) == "running");
        f.finish(id, "CLOSE", "closed");
        CHECK(f.facade.getStatus(id) == "completed");
    }

    TEST_CASE("Education: the bursary amount picks the approval level") {
        Fixture f;
        const std::vector<std::pair<std::string, std::string>> cases = {
            {"50000", "aid"}, {"50001", "mgr"}, {"150000", "mgr"}, {"150001", "dean"}};

        for (const auto& c : cases) {
            const std::string id = f.facade.startInstance(f.education, educationData(c.first));
            f.finish(id, "CAPTURE", "captured");
            f.finish(id, "VERIFY_ID", "verified");
            f.finish(id, "REVIEW", "admit");
            f.finish(id, "OFFER", "bursary requested");
            CHECK(f.item(id, "BURSARY").assigneeId == c.second);
        }
    }

    TEST_CASE("Education: declining takes the regret branch and skips the offer path") {
        Fixture f;
        const std::string id = f.facade.startInstance(f.education, educationData());
        f.finish(id, "CAPTURE", "captured");
        f.finish(id, "VERIFY_ID", "verified");
        f.finish(id, "REVIEW", "decline");

        CHECK(f.state(id, "OFFER") == "Cancelled");
        CHECK(f.state(id, "BURSARY") == "Cancelled");
        CHECK(f.state(id, "PAY") == "Cancelled");
        CHECK(f.state(id, "REGRET") == "Assigned");
        CHECK(f.state(id, "CLOSE") == "Created"); // waiting on the regret letter

        f.finish(id, "REGRET", "sent");
        f.finish(id, "CLOSE", "closed");
        CHECK(f.facade.getStatus(id) == "completed");
    }

    TEST_CASE("Education: a rejected identity check terminates the instance") {
        Fixture f;
        InstanceData data = educationData();
        data.set("surname", "Wrong");
        const std::string id = f.facade.startInstance(f.education, data);
        f.finish(id, "CAPTURE", "captured");

        REQUIRE(f.facade.start(id, "VERIFY_ID", f.who("clerk")));
        CHECK(f.facade.getInstanceData(id)["identityStatus"] == "SURNAME_MISMATCH");
        REQUIRE(f.facade.reject(id, "VERIFY_ID", f.who("clerk"), "surname mismatch"));

        CHECK(f.state(id, "INTAKE") == "Rejected");
        CHECK(f.state(id, "REVIEW") == "Cancelled");
        CHECK(f.facade.getStatus(id) == "terminated");
    }

    TEST_CASE("Logistics: recovery runs carrier and customer work in parallel, then joins") {
        Fixture f;
        const std::string id = f.facade.startInstance(f.logistics, logisticsData());
        f.finish(id, "LOG", "logged");

        REQUIRE(f.facade.start(id, "TRACK", f.who("wh")));
        CHECK(f.facade.getInstanceData(id)["shipmentStatus"] == "DELAYED");
        REQUIRE(f.facade.complete(id, "TRACK", f.who("wh"), "checked"));

        CHECK(f.item(id, "ASSESS").assigneeId == "analyst"); // severity high
        f.finish(id, "ASSESS", "reroute");

        CHECK(f.state(id, "REROUTE") == "Assigned");
        CHECK(f.state(id, "NOTIFY") == "Assigned");
        CHECK(f.state(id, "CLAIM") == "Cancelled");

        f.finish(id, "NOTIFY", "informed");
        CHECK(f.state(id, "CLOSE") == "Created");

        REQUIRE(f.facade.start(id, "REROUTE", f.who("analyst")));
        CHECK(f.facade.getInstanceData(id)["newHub"] == "Cape Town");
        REQUIRE(f.facade.complete(id, "REROUTE", f.who("analyst"), "rebooked"));

        CHECK(f.state(id, "RECOVERY") == "Completed");
        f.finish(id, "CLOSE", "closed");
        CHECK(f.facade.getStatus(id) == "completed");
    }

    TEST_CASE("Logistics: no participant for the role escalates until someone claims") {
        Fixture f;
        const std::string id = f.facade.startInstance(f.logistics, logisticsData("critical"));
        f.finish(id, "LOG", "logged");
        f.finish(id, "TRACK", "checked");

        CHECK(f.state(id, "ASSESS") == "Escalated");
        CHECK(f.item(id, "ASSESS").escalationLevel == 1);
        CHECK(f.item(id, "ASSESS").assigneeId.empty());

        REQUIRE(f.facade.claim(id, "ASSESS", f.who("sup")));
        CHECK(f.state(id, "ASSESS") == "Assigned");
        CHECK(f.item(id, "ASSESS").assigneeId == "sup");
    }

    TEST_CASE("Logistics: a failing service escalates and can be retried after claiming") {
        Fixture f;
        const std::string id = f.facade.startInstance(f.logistics, logisticsData("high", "Polokwane"));
        f.finish(id, "LOG", "logged");
        f.finish(id, "TRACK", "checked");
        f.finish(id, "ASSESS", "reroute");

        REQUIRE(f.facade.start(id, "REROUTE", f.who("analyst")));
        CHECK(f.state(id, "REROUTE") == "Escalated");
        CHECK(f.item(id, "REROUTE").serviceMessage.find("Polokwane") != std::string::npos);

        REQUIRE(f.facade.claim(id, "REROUTE", f.who("sup")));
        CHECK(f.state(id, "REROUTE") == "Assigned");
    }

    TEST_CASE("Instances of one definition are independent") {
        Fixture f;
        const std::string first = f.facade.startInstance(f.education, educationData());
        const std::string second = f.facade.startInstance(f.education, educationData());
        REQUIRE(first != second);

        f.finish(first, "CAPTURE", "captured");

        CHECK(f.state(first, "CAPTURE") == "Completed");
        CHECK(f.state(second, "CAPTURE") == "Assigned");
        CHECK(f.state(second, "VERIFY_ID") == "Created");
    }

    TEST_CASE("Invalid actions are refused and change nothing") {
        Fixture f;
        const std::string id = f.facade.startInstance(f.education, educationData());

        // Wrong participant
        CHECK_FALSE(f.facade.start(id, "CAPTURE", f.who("officer")));
        // Completing before starting
        CHECK_FALSE(f.facade.complete(id, "CAPTURE", f.who("clerk"), "captured"));
        // Claiming work that already has an assignee
        CHECK_FALSE(f.facade.claim(id, "CAPTURE", f.who("officer")));
        // Delegating someone else's work
        CHECK_FALSE(f.facade.delegate(id, "CAPTURE", f.who("officer"), f.who("officer")));
        // Work whose dependencies are not met
        CHECK_FALSE(f.facade.start(id, "VERIFY_ID", f.who("clerk")));
        // Unknown item
        CHECK_FALSE(f.facade.start(id, "NOPE", f.who("clerk")));

        CHECK(f.state(id, "CAPTURE") == "Assigned");
        CHECK(f.item(id, "CAPTURE").assigneeId == "clerk");
    }

    TEST_CASE("Delegation moves the work to the new assignee") {
        Fixture f;
        const std::string id = f.facade.startInstance(f.education, educationData());

        REQUIRE(f.facade.delegate(id, "CAPTURE", f.who("clerk"), f.who("officer")));
        CHECK(f.item(id, "CAPTURE").assigneeId == "officer");
        CHECK_FALSE(f.facade.start(id, "CAPTURE", f.who("clerk")));
        CHECK(f.facade.start(id, "CAPTURE", f.who("officer")));
    }

    TEST_CASE("Cancelling the root terminates the instance") {
        Fixture f;
        const std::string id = f.facade.startInstance(f.education, educationData());

        REQUIRE(f.facade.cancel(id, "ADMISSIONS", f.who("clerk")));
        CHECK(f.facade.getStatus(id) == "terminated");
        CHECK(f.state(id, "CAPTURE") == "Cancelled");
    }

    TEST_CASE("Undo restores state, and the workflow can carry on afterwards") {
        Fixture f;
        const std::string id = f.facade.startInstance(f.education, educationData());
        f.finish(id, "CAPTURE", "captured");
        REQUIRE(f.state(id, "VERIFY_ID") == "Assigned");

        REQUIRE(f.facade.undoLast());
        CHECK(f.state(id, "CAPTURE") == "InProgress");
        CHECK(f.state(id, "VERIFY_ID") == "Created");
        CHECK(f.item(id, "CAPTURE").assigneeId == "clerk");

        // The item that was rolled back must open again on the second completion
        REQUIRE(f.facade.complete(id, "CAPTURE", f.who("clerk"), "captured"));
        CHECK(f.state(id, "VERIFY_ID") == "Assigned");
        CHECK(f.item(id, "VERIFY_ID").assigneeId == "clerk");
    }

    TEST_CASE("Undo restores data written by a service") {
        Fixture f;
        const std::string id = f.facade.startInstance(f.education, educationData());
        f.finish(id, "CAPTURE", "captured");
        REQUIRE(f.facade.start(id, "VERIFY_ID", f.who("clerk")));
        REQUIRE(f.facade.getInstanceData(id).count("identityVerified") == 1);

        REQUIRE(f.facade.undoLast());
        CHECK(f.facade.getInstanceData(id).count("identityVerified") == 0);
    }

    TEST_CASE("Queries on an unknown instance return nothing") {
        Fixture f;
        CHECK(f.facade.getWorkItems("missing").empty());
        CHECK(f.facade.getInstanceData("missing").empty());
        CHECK(f.facade.findParticipant("missing") == nullptr);
    }
}

TEST_SUITE("Decorators and deadlines in running instances") {

    /// Brings an education instance to the bursary approval step.
    std::string toBursary(Fixture& f, const std::string& amount = "45000") {
        const std::string id = f.facade.startInstance(f.education, educationData(amount));
        f.finish(id, "CAPTURE", "captured");
        f.finish(id, "VERIFY_ID", "verified");
        f.finish(id, "REVIEW", "admit");
        f.finish(id, "OFFER", "bursary requested");
        return id;
    }

    const std::time_t LATER = std::time(nullptr) + 49 * 3600;

    TEST_CASE("A decorated item runs like any other and reports its priority") {
        Fixture f;
        const std::string id = toBursary(f);

        CHECK(f.item(id, "BURSARY").priority == 2);
        CHECK(f.item(id, "CAPTURE").priority == 0);
        CHECK(f.state(id, "BURSARY") == "Assigned");

        // Priority is not an escalation, so the chain still starts at its first level
        CHECK(f.item(id, "BURSARY").escalationLevel == 0);
        CHECK(f.item(id, "BURSARY").assigneeId == "aid");

        f.finish(id, "BURSARY", "approved");
        CHECK(f.state(id, "BURSARY") == "Completed");
        CHECK(f.state(id, "PAY") == "Assigned");
    }

    TEST_CASE("AuditDecorator records attempts under the instance, including refused ones") {
        Fixture f;
        const std::string id = toBursary(f);
        const size_t before = f.facade.getAuditedAttempts().size();

        CHECK_FALSE(f.facade.claim(id, "BURSARY", f.who("mgr"))); // the action refuses first
        REQUIRE(f.facade.start(id, "BURSARY", f.who("aid")));
        CHECK_FALSE(f.facade.start(id, "BURSARY", f.who("aid"))); // already started: the state refuses

        const std::vector<std::string>& attempts = f.facade.getAuditedAttempts();
        REQUIRE(attempts.size() == before + 2);
        CHECK(attempts.back().find("[INSTANCE:" + id + "]") != std::string::npos);
        CHECK(attempts.back().find("BURSARY") != std::string::npos);
    }

    TEST_CASE("Undo and the mediator work through decorated items") {
        Fixture f;
        const std::string id = toBursary(f);
        f.finish(id, "BURSARY", "approved");
        REQUIRE(f.state(id, "PAY") == "Assigned");

        REQUIRE(f.facade.undoLast());
        CHECK(f.state(id, "BURSARY") == "InProgress");
        CHECK(f.state(id, "PAY") == "Created");

        REQUIRE(f.facade.complete(id, "BURSARY", f.who("aid"), "approved"));
        CHECK(f.state(id, "PAY") == "Assigned");
    }

    TEST_CASE("Decorating one instance's tree leaves the next instance independent") {
        Fixture f;
        const std::string first = toBursary(f);
        const std::string second = f.facade.startInstance(f.education, educationData());

        f.finish(first, "BURSARY", "approved");

        CHECK(f.state(first, "BURSARY") == "Completed");
        CHECK(f.state(second, "BURSARY") == "Created");
        CHECK(f.item(second, "BURSARY").priority == 2);
    }

    TEST_CASE("Nothing escalates before its deadline") {
        Fixture f;
        const std::string id = toBursary(f);

        f.facade.checkDeadlines(std::time(nullptr) + 47 * 3600);

        CHECK(f.state(id, "BURSARY") == "Assigned");
        CHECK(f.item(id, "BURSARY").escalationLevel == 0);
    }

    TEST_CASE("A missed deadline walks up the approval chain, then waits to be claimed") {
        Fixture f;
        const std::string id = toBursary(f);
        REQUIRE(f.item(id, "BURSARY").assigneeId == "aid");

        f.facade.checkDeadlines(LATER);
        CHECK(f.state(id, "BURSARY") == "Assigned");
        CHECK(f.item(id, "BURSARY").assigneeId == "mgr");
        CHECK(f.item(id, "BURSARY").escalationLevel == 1);

        f.facade.checkDeadlines(LATER);
        CHECK(f.item(id, "BURSARY").assigneeId == "dean");

        // Nobody is left above the Dean
        f.facade.checkDeadlines(LATER);
        CHECK(f.state(id, "BURSARY") == "Escalated");
        CHECK(f.item(id, "BURSARY").assigneeId.empty());

        REQUIRE(f.facade.claim(id, "BURSARY", f.who("dean")));
        f.finish(id, "BURSARY", "approved");
        CHECK(f.state(id, "PAY") == "Assigned");
    }

    TEST_CASE("A missed deadline with no one else to route to stays escalated") {
        Fixture f;
        const std::string id = f.facade.startInstance(f.logistics, logisticsData());
        f.finish(id, "LOG", "logged");
        f.finish(id, "TRACK", "checked");
        REQUIRE(f.item(id, "ASSESS").assigneeId == "analyst");

        f.facade.checkDeadlines(std::time(nullptr) + 25 * 3600);

        CHECK(f.state(id, "ASSESS") == "Escalated");
        CHECK(f.item(id, "ASSESS").outcome == "Deadline passed");
    }

    TEST_CASE("Finished work and unopened work ignore deadlines") {
        Fixture f;
        const std::string finished = toBursary(f);
        f.finish(finished, "BURSARY", "approved");
        const std::string unopened = f.facade.startInstance(f.education, educationData());

        f.facade.checkDeadlines(LATER);

        CHECK(f.state(finished, "BURSARY") == "Completed");
        CHECK(f.state(unopened, "BURSARY") == "Created");
    }

    TEST_CASE("Undo restores the escalation count") {
        Fixture f;
        const std::string id = f.facade.startInstance(f.logistics, logisticsData("critical"));
        f.finish(id, "LOG", "logged");
        f.finish(id, "TRACK", "checked");
        REQUIRE(f.state(id, "ASSESS") == "Escalated");
        REQUIRE(f.item(id, "ASSESS").escalationLevel == 1);

        REQUIRE(f.facade.undoLast());
        CHECK(f.state(id, "ASSESS") == "Created");
        CHECK(f.item(id, "ASSESS").escalationLevel == 0);
    }
}
