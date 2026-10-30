/**
 * @file test_mediator_cor_iterator.cpp
 * @brief Unit tests for Mediator, Chain of Responsibility, and Iterator patterns.
 * @author Angelo Anthony
 */

#include "doctest.h"

#include <vector>
#include <string>
#include <memory>

#include "WorkItem.h"
#include "Task.h"
#include "SubProcess.h"
#include "Iterator.h"
#include "WorkItemIterator.h"
#include "DepthFirstWorkItemIterator.h"
#include "WorkflowMediator.h"
#include "DependencyMediator.h"
#include "DependencyRule.h"
#include "ApprovalHandler.h"
#include "ApprovalRequest.h"
#include "ConcreteApprovalHandler.h"
#include "ApprovalChainIterator.h"
#include "ThresholdRule.h"
#include "AlwaysAcceptRule.h"
#include "ApprovalRouting.h"
#include "RoleBasedRouting.h"
#include "Participant.h"
#include "ParticipantDirectory.h"
#include "InstanceData.h"

static const Task dummyTask("DUMMY", "Dummy Task");


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
        : root(dynamic_cast<SubProcess*>(definitionRoot.clone())), mediator(rules) {
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

TEST_SUITE("Angelo Anthony - Chain of Responsibility Subsystem") {

    TEST_CASE("CoR: each request goes to the lowest level with enough authority") {
        ParticipantDirectory directory;
        addBursaryApprovers(directory);
        std::unique_ptr<ApprovalHandler> chain(buildBursaryChain());

        InstanceData small, medium, large;
        small.set("amount", "15000");
        medium.set("amount", "65000");
        large.set("amount", "250000");

        ApprovalRequest r1(nullptr, &small, 0, &directory);
        ApprovalRequest r2(nullptr, &medium, 0, &directory);
        ApprovalRequest r3(nullptr, &large, 0, &directory);
        CHECK(chain->handle(r1)->getName() == "Zanele Dube");
        CHECK(chain->handle(r2)->getName() == "Michael Smith");
        CHECK(chain->handle(r3)->getName() == "Prof. Naidoo");
    }

    TEST_CASE("CoR: a missing or non-numeric amount is passed up to unlimited authority") {
        ParticipantDirectory directory;
        addBursaryApprovers(directory);
        std::unique_ptr<ApprovalHandler> chain(buildBursaryChain());

        InstanceData missing, garbage;
        garbage.set("amount", "R15k");
        ApprovalRequest r1(nullptr, &missing, 0, &directory);
        ApprovalRequest r2(nullptr, &garbage, 0, &directory);
        CHECK(chain->handle(r1)->getName() == "Prof. Naidoo");
        CHECK(chain->handle(r2)->getName() == "Prof. Naidoo");
    }

    TEST_CASE("CoR: a level with nobody in its role passes the request on") {
        ParticipantDirectory directory;
        directory.addParticipant(new Participant("B2", "Michael Smith", "FinanceManager"));
        std::unique_ptr<ApprovalHandler> chain(buildBursaryChain());

        InstanceData data;
        data.set("amount", "5000");
        ApprovalRequest request(nullptr, &data, 0, &directory);
        CHECK(chain->handle(request)->getName() == "Michael Smith");
    }

    TEST_CASE("CoR: the end of the chain returns nullptr when nobody can approve") {
        ParticipantDirectory directory;
        addBursaryApprovers(directory);
        ConcreteApprovalHandler officer("BursaryOfficer", new ThresholdRule("amount", 20000));

        InstanceData data;
        data.set("amount", "50000");
        ApprovalRequest request(nullptr, &data, 0, &directory);
        CHECK(officer.handle(request) == nullptr);

        ApprovalRequest noDirectory(nullptr, &data, 0, nullptr);
        CHECK(officer.handle(noDirectory) == nullptr);
    }

    TEST_CASE("CoR: escalating an approval task hands it to the next level") {
        ParticipantDirectory directory;
        addBursaryApprovers(directory);
        InstanceData data;
        data.set("amount", "12000");

        Task approve("APPROVE", "Approve Bursary");
        approve.setRouting(new ApprovalRouting(buildBursaryChain()));
        approve.setRoutingContext(&data, &directory);

        REQUIRE(approve.makeAvailable());
        CHECK(approve.getStateName() == "Assigned");
        CHECK(approve.getAssignee()->getName() == "Zanele Dube");

        // Deadline passed at the officer: escalate, then route to the next level.
        REQUIRE(approve.escalate());
        CHECK(approve.getEscalationLevel() == 1);
        CHECK(approve.getAssignee() == nullptr);
        REQUIRE(approve.route());
        CHECK(approve.getStateName() == "Assigned");
        CHECK(approve.getAssignee()->getName() == "Michael Smith");

        REQUIRE(approve.escalate());
        REQUIRE(approve.route());
        CHECK(approve.getAssignee()->getName() == "Prof. Naidoo");

        REQUIRE(approve.start());
        CHECK(approve.complete("approved"));
    }

    TEST_CASE("CoR: a task nobody can approve escalates automatically when it opens") {
        ParticipantDirectory directory;
        addBursaryApprovers(directory);
        InstanceData data;
        data.set("amount", "50000");

        Task approve("APPROVE", "Approve Bursary");
        approve.setRouting(new ApprovalRouting(
            new ConcreteApprovalHandler("BursaryOfficer", new ThresholdRule("amount", 20000))));
        approve.setRoutingContext(&data, &directory);

        REQUIRE(approve.makeAvailable());
        CHECK(approve.getStateName() == "Escalated");
        CHECK(approve.getAssignee() == nullptr);
        CHECK(approve.getEscalationLevel() == 1);
    }

    TEST_CASE("CoR: cloned routing carries its own copy of the whole chain") {
        ParticipantDirectory directory;
        addBursaryApprovers(directory);
        InstanceData data;
        data.set("amount", "65000");

        RoutingStrategy* copy = nullptr;
        {
            ApprovalRouting original(buildBursaryChain());
            copy = original.clone();
            ApprovalRouting* approvalCopy = static_cast<ApprovalRouting*>(copy);
            CHECK(approvalCopy->getChain() != original.getChain());
            CHECK(approvalCopy->describeChain() == original.describeChain());
        } // original and its chain are destroyed here

        Participant* approver = copy->select(dummyTask, data, directory);
        REQUIRE(approver != nullptr);
        CHECK(approver->getName() == "Michael Smith");
        delete copy;
    }

    TEST_CASE("CoR: an approval task cloned from a definition still routes through the chain") {
        ParticipantDirectory directory;
        addBursaryApprovers(directory);

        SubProcess definition("FUNDING", "Bursary Funding");
        Task* approve = new Task("APPROVE", "Approve Bursary");
        approve->setRouting(new ApprovalRouting(buildBursaryChain()));
        definition.add(approve);

        TestInstance inst(definition, {}, directory);
        inst.data.set("amount", "250000");
        REQUIRE(inst.root->makeAvailable());
        CHECK(inst.state("APPROVE") == "Assigned");
        CHECK(inst.item("APPROVE")->getAssignee()->getName() == "Prof. Naidoo");
    }

    TEST_CASE("CoR: setNext returns its argument and replaces the old successor") {
        ConcreteApprovalHandler head("BursaryOfficer", new ThresholdRule("amount", 20000));
        ApprovalHandler* manager = new ConcreteApprovalHandler("FinanceManager", new AlwaysAcceptRule());
        CHECK(head.setNext(manager) == manager);
        CHECK(head.getNext() == manager);

        ApprovalHandler* committee = new ConcreteApprovalHandler("BursaryCommittee", new AlwaysAcceptRule());
        head.setNext(committee); // deletes the manager handler
        CHECK(head.getNext() == committee);

        ApprovalRouting routing;
        CHECK(routing.describeChain().empty());
        CHECK(routing.select(dummyTask, InstanceData(), ParticipantDirectory()) == nullptr);
    }
}
