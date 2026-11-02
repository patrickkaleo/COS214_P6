/**
 * @file test_scenario_rosters.cpp
 * @brief Tests that each roster owns its participants, so one context cannot affect another.
 * @author Angela Ramaboea
 */

#include "doctest.h"

#include "Participant.h"
#include "TividyWorkflowFacade.h"
#include "WorkItem.h"
#include "WorkflowInstance.h"

#include <string>

namespace {

const char* const WORKFLOW = "definitions/shipment_exception.definition";

/// Two rosters with the same id and role, but separate people.
struct Rosters {
    TividyWorkflowFacade facade;
    std::string definition;

    Rosters() {
        definition = facade.registerDefinitionFromFile(WORKFLOW);
        facade.addParticipant("clerk", "Bongani Dube", {"WarehouseClerk"}, "first");
        facade.addParticipant("clerk", "Bongani Dube", {"WarehouseClerk"}, "second");
    }

    Participant* assigneeOf(const std::string& instance, const std::string& item) {
        return facade.getInstance(instance)->findItem(item)->getAssignee();
    }
};

} // namespace

TEST_SUITE("Scenario rosters") {

    TEST_CASE("The same id and name in two rosters are different participants") {
        Rosters r;
        REQUIRE(r.facade.getLastError().empty());
        Participant* first = r.facade.findParticipant("clerk", "first");
        Participant* second = r.facade.findParticipant("clerk", "second");

        REQUIRE(first != nullptr);
        REQUIRE(second != nullptr);
        CHECK(first != second);
        CHECK(first->getName() == second->getName());
    }

    TEST_CASE("Work routes only to the instance's own roster") {
        Rosters r;
        const std::string a = r.facade.startInstance(r.definition, InstanceData(), "first");
        const std::string b = r.facade.startInstance(r.definition, InstanceData(), "second");

        CHECK(r.assigneeOf(a, "LOG_EXCEPTION") == r.facade.findParticipant("clerk", "first"));
        CHECK(r.assigneeOf(b, "LOG_EXCEPTION") == r.facade.findParticipant("clerk", "second"));
    }

    TEST_CASE("A participant from another roster cannot act on the work") {
        Rosters r;
        const std::string a = r.facade.startInstance(r.definition, InstanceData(), "first");
        Participant* outsider = r.facade.findParticipant("clerk", "second");

        CHECK(r.facade.availableActions(a, "LOG_EXCEPTION", outsider).empty());
        CHECK_FALSE(r.facade.start(a, "LOG_EXCEPTION", outsider));
        CHECK(r.facade.start(a, "LOG_EXCEPTION", r.facade.findParticipant("clerk", "first")));
    }

    TEST_CASE("A role held only in another roster escalates") {
        Rosters r;
        r.facade.addParticipant("other", "Someone Else", {"Manager"}, "third");
        const std::string id = r.facade.startInstance(r.definition, InstanceData(), "third");

        CHECK(r.facade.getInstance(id)->findItem("LOG_EXCEPTION")->getStateName() == "Escalated");
    }

    TEST_CASE("An id can only be used once inside a roster") {
        Rosters r;
        r.facade.addParticipant("clerk", "Duplicate", {"WarehouseClerk"}, "first");

        CHECK_FALSE(r.facade.getLastError().empty());
        CHECK(r.facade.findParticipant("clerk", "first")->getName() == "Bongani Dube");
    }

    TEST_CASE("Undo restores an assignee from the instance's own roster") {
        Rosters r;
        const std::string id = r.facade.startInstance(r.definition, InstanceData(), "second");
        Participant* clerk = r.facade.findParticipant("clerk", "second");
        REQUIRE(r.facade.start(id, "LOG_EXCEPTION", clerk));

        REQUIRE(r.facade.undoLast());
        CHECK(r.assigneeOf(id, "LOG_EXCEPTION") == clerk);
    }

    TEST_CASE("Without a roster the shared directory still works as before") {
        TividyWorkflowFacade facade;
        const std::string definition = facade.registerDefinitionFromFile(WORKFLOW);
        facade.addParticipant("clerk", "Shared Clerk", {"WarehouseClerk"});
        const std::string id = facade.startInstance(definition, InstanceData());

        CHECK(facade.getInstance(id)->findItem("LOG_EXCEPTION")->getAssignee() == facade.findParticipant("clerk"));
        CHECK(facade.findParticipant("clerk", "unknown") == nullptr);
    }
}
