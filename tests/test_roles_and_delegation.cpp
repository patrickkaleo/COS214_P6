/**
 * @file test_roles_and_delegation.cpp
 * @brief Tests participation in the prepared workflows: roles, delegation, approval levels and commands.
 * @author Angela Ramaboea
 */

#include "doctest.h"

#include "Participant.h"
#include "TividyWorkflowFacade.h"
#include "PriorityDecorator.h"
#include "WorkItem.h"
#include "WorkItemDecorator.h"
#include "WorkflowInstance.h"
#include "BankGatewayAdapter.h"
#include "CarrierAdapter.h"
#include "GovernmentIDAdapter.h"
#include "TrackingServiceAdapter.h"

#include <cstdio>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>

namespace {

struct University {
    TividyWorkflowFacade facade;
    std::string definition;

    University() {
        definition = facade.registerDefinitionFromFile("definitions/university_bursary.txt");
        facade.addService(new GovernmentIDAdapter());
        facade.addService(new BankGatewayAdapter());
        facade.addService(new TrackingServiceAdapter());
        facade.addService(new CarrierAdapter());
        facade.addParticipant("admissions", "Admissions One", {"AdmissionsOfficer"});
        facade.addParticipant("admissions2", "Admissions Two", {"AdmissionsOfficer"});
        facade.addParticipant("reviewer", "Reviewer", {"FacultyReviewer"});
        facade.addParticipant("engineer", "Engineer", {"EngineeringReviewer"});
        facade.addParticipant("committee", "Committee", {"BursaryCommittee"});
        facade.addParticipant("officer", "Officer", {"BursaryOfficer"});
        facade.addParticipant("finance", "Finance", {"FinanceManager"});
        facade.addParticipant("admin", "Admin", {"WorkflowAdmin"});
    }

    Participant* who(const std::string& id) { return facade.findParticipant(id); }

    WorkItem* item(const std::string& instance, const std::string& id) {
        return facade.getInstance(instance)->findItem(id);
    }

    std::string start(const std::string& amount = "15000", const std::string& degree = "Law") {
        InstanceData data;
        data.set("idNumber", "0501015800087");
        data.set("surname", "Mokoena");
        data.set("degree", degree);
        data.set("amount", amount);
        data.set("accountNumber", "62012345678");
        data.set("branchCode", "250655");
        data.set("reference", "TEST");
        return facade.startInstance(definition, data);
    }

    void finish(const std::string& instance, const std::string& id, const std::string& outcome = "done") {
        Participant* assignee = item(instance, id)->getAssignee();
        REQUIRE(assignee != nullptr);
        REQUIRE(facade.start(instance, id, assignee));
        REQUIRE(facade.complete(instance, id, assignee, outcome));
    }

    std::string atApproval(const std::string& amount) {
        const std::string id = start(amount);
        finish(id, "CAPTURE");
        finish(id, "VERIFY_ID");
        finish(id, "VERIFY_DOCS");
        finish(id, "ACADEMIC");
        finish(id, "DECIDE", "approved");
        finish(id, "FUNDING_DOCS");
        return id;
    }
};

} // namespace

TEST_SUITE("Roles, delegation and commands") {

    TEST_CASE("Routing assigns the role the definition names") {
        University u;
        const std::string id = u.start();
        CHECK(u.item(id, "CAPTURE")->getAssignee() == u.who("admissions"));
        CHECK(u.item(id, "CAPTURE")->getStateName() == "Assigned");
    }

    TEST_CASE("Data routing follows the details entered for the instance") {
        University u;
        for (const auto& c : std::vector<std::pair<std::string, std::string>>{{"Law", "reviewer"}, {"Engineering", "engineer"}}) {
            const std::string id = u.start("15000", c.first);
            u.finish(id, "CAPTURE");
            u.finish(id, "VERIFY_ID");
            CHECK(u.item(id, "ACADEMIC")->getAssignee() == u.who(c.second));
        }
    }

    TEST_CASE("Delegation works within a role and moves the work") {
        University u;
        const std::string id = u.start();
        REQUIRE(u.facade.delegate(id, "CAPTURE", u.who("admissions"), u.who("admissions2")));

        CHECK(u.item(id, "CAPTURE")->getAssignee() == u.who("admissions2"));
        CHECK(u.item(id, "CAPTURE")->getStateName() == "Assigned");
        CHECK_FALSE(u.facade.start(id, "CAPTURE", u.who("admissions")));
        CHECK(u.facade.start(id, "CAPTURE", u.who("admissions2")));
    }

    TEST_CASE("Delegation is refused to the wrong role, to yourself, and by a non-assignee") {
        University u;
        const std::string id = u.start();

        CHECK_FALSE(u.facade.delegate(id, "CAPTURE", u.who("admissions"), u.who("reviewer")));
        CHECK_FALSE(u.facade.delegate(id, "CAPTURE", u.who("admissions"), u.who("admissions")));
        CHECK_FALSE(u.facade.delegate(id, "CAPTURE", u.who("admissions2"), u.who("admissions2")));
        CHECK(u.item(id, "CAPTURE")->getAssignee() == u.who("admissions"));
    }

    TEST_CASE("Delegation is refused once work has started") {
        University u;
        const std::string id = u.start();
        REQUIRE(u.facade.start(id, "CAPTURE", u.who("admissions")));

        CHECK_FALSE(u.facade.delegate(id, "CAPTURE", u.who("admissions"), u.who("admissions2")));
        CHECK(u.item(id, "CAPTURE")->getStateName() == "InProgress");
    }

    TEST_CASE("Only the assignee can start, complete or reject") {
        University u;
        const std::string id = u.start();
        CHECK_FALSE(u.facade.start(id, "CAPTURE", u.who("admissions2")));
        REQUIRE(u.facade.start(id, "CAPTURE", u.who("admissions")));
        CHECK_FALSE(u.facade.complete(id, "CAPTURE", u.who("admissions2"), "done"));
        CHECK_FALSE(u.facade.reject(id, "CAPTURE", u.who("admissions2"), "no"));
    }

    TEST_CASE("Available actions match what the engine then accepts") {
        University u;
        const std::string id = u.start();

        const std::vector<std::string> assignee = u.facade.availableActions(id, "CAPTURE", u.who("admissions"));
        CHECK(std::vector<std::string>{"start", "delegate", "cancel"} == assignee);
        CHECK(u.facade.availableActions(id, "CAPTURE", u.who("reviewer")).empty());
    }

    TEST_CASE("Only a qualified participant can claim escalated work") {
        University u;
        const std::string id = u.atApproval("15000");
        REQUIRE(u.item(id, "APPROVE")->getAssignee() == u.who("officer"));
        u.facade.checkDeadlines(std::time(nullptr) + 49 * 3600);
        REQUIRE(u.item(id, "APPROVE")->getAssignee() == u.who("finance"));

        CHECK_FALSE(u.facade.claim(id, "APPROVE", u.who("reviewer")));
    }

    TEST_CASE("A priority task has not been escalated") {
        University u;
        const std::string id = u.start();
        WorkItem* pay = u.item(id, "PAY");

        PriorityDecorator* priority = nullptr;
        for (WorkItem* layer = pay; priority == nullptr && dynamic_cast<WorkItemDecorator*>(layer) != nullptr;
             layer = dynamic_cast<WorkItemDecorator*>(layer)->getInner()) {
            priority = dynamic_cast<PriorityDecorator*>(layer);
        }
        REQUIRE(priority != nullptr);
        CHECK(priority->getPriority() == 2);
        CHECK(pay->getEscalationLevel() == 0);
    }

    TEST_CASE("The bursary amount picks the approval level") {
        University u;
        CHECK(u.item(u.atApproval("50000"), "APPROVE")->getAssignee() == u.who("officer"));
        CHECK(u.item(u.atApproval("50001"), "APPROVE")->getAssignee() == u.who("finance"));
        CHECK(u.item(u.atApproval("150001"), "APPROVE")->getAssignee() == u.who("committee"));
    }

    TEST_CASE("Missed deadlines walk up the chain and stop at the top") {
        University u;
        const std::string id = u.atApproval("15000");
        WorkItem* approve = u.item(id, "APPROVE");
        WorkflowInstance* instance = u.facade.getInstance(id);

        // Each escalation starts a new deadline, counted from the moment it happened
        u.facade.checkDeadlines(instance->getDeadline("APPROVE"));
        CHECK(approve->getAssignee() == u.who("finance"));
        u.facade.checkDeadlines(instance->getDeadline("APPROVE"));
        CHECK(approve->getAssignee() == u.who("committee"));

        // Nobody is above the committee: it keeps the work rather than stranding it
        u.facade.checkDeadlines(instance->getDeadline("APPROVE"));
        CHECK(approve->getAssignee() == u.who("committee"));
        CHECK(approve->getStateName() == "Assigned");

        REQUIRE(u.facade.start(id, "APPROVE", u.who("committee")));
        CHECK(u.facade.complete(id, "APPROVE", u.who("committee"), "approved"));
    }

    TEST_CASE("Only commands that ran are kept, in order, and undo reverses the newest") {
        University u;
        const std::string id = u.start();
        REQUIRE(u.facade.actionHistory().empty());

        REQUIRE(u.facade.start(id, "CAPTURE", u.who("admissions")));
        CHECK_FALSE(u.facade.start(id, "CAPTURE", u.who("admissions")));
        REQUIRE(u.facade.complete(id, "CAPTURE", u.who("admissions"), "done"));

        std::vector<std::string> history = u.facade.actionHistory();
        REQUIRE(history.size() == 2);
        CHECK(history[0].find("StartAction: CAPTURE") != std::string::npos);
        CHECK(history[1].find("CompleteAction: CAPTURE") != std::string::npos);

        REQUIRE(u.facade.undoLast());
        CHECK(u.item(id, "CAPTURE")->getStateName() == "InProgress");
        CHECK(u.item(id, "VERIFY_ID")->getStateName() == "Created");
        REQUIRE(u.facade.actionHistory().size() == 1);

        REQUIRE(u.facade.undoLast());
        CHECK(u.item(id, "CAPTURE")->getStateName() == "Assigned");
        CHECK(u.item(id, "CAPTURE")->getAssignee() == u.who("admissions"));
    }

    TEST_CASE("Undoing a delegation gives the work back") {
        University u;
        const std::string id = u.start();
        REQUIRE(u.facade.delegate(id, "CAPTURE", u.who("admissions"), u.who("admissions2")));
        REQUIRE(u.facade.undoLast());
        CHECK(u.item(id, "CAPTURE")->getAssignee() == u.who("admissions"));
    }

    TEST_CASE("Only a workflow admin can cancel a whole stage") {
        University u;
        const std::string id = u.start();
        CHECK_FALSE(u.facade.cancel(id, "UNI_APP", u.who("admissions")));
        CHECK(u.facade.cancel(id, "UNI_APP", u.who("admin")));
        CHECK(u.facade.getStatus(id) == "terminated");
    }

    TEST_CASE("A malformed approval chain in a definition is reported") {
        const std::string path = "tests/approval_test.tmp";
        {
            std::ofstream file(path.c_str());
            file << "DEFINITION\nBAD_CHAIN|Bad|1\nWORKITEMS\nprocess|R|-|Root\ntask|A|R|A\n"
                 << "ROUTING\nA|approval|amount|Officer|not-a-number|Manager\n";
        }
        TividyWorkflowFacade facade;
        const std::string id = facade.registerDefinitionFromFile(path);
        std::remove(path.c_str());

        CHECK(id.empty());
        CHECK(facade.getLastError().find("approval limit") != std::string::npos);
    }
}
