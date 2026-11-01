#include "doctest.h"
#include "TividyWorkflowFacade.h"
#include "WorkflowDefinitionBuilder.h"
#include "WorkflowInstance.h"
#include "WorkflowEngine.h"
#include "Task.h"
#include "Participant.h"
#include "RoleBasedRouting.h"
#include "ApprovalRouting.h"
#include "ConcreteApprovalHandler.h"
#include "ThresholdRule.h"
#include "AlwaysAcceptRule.h"
#include "StartAction.h"
#include "CompleteAction.h"
#include "RejectAction.h"
#include "ExternalService.h"
#include "ServiceRequest.h"
#include "ServiceResult.h"
#include <algorithm>
#include <memory>
#include <limits>

namespace {
class RuntimeLinearBuilder : public WorkflowDefinitionBuilder {
    std::string id;
    bool decorated;
    int deadlineHours;
    bool service;
public:
    RuntimeLinearBuilder(const std::string &id, bool decorated = false,
                         int deadlineHours = 0, bool service = false)
        : id(id), decorated(decorated), deadlineHours(deadlineHours), service(service) {}
    void buildStructure() override {
        startDefinition(id, "Runtime regression", 1);
        addProcess("ROOT", "Not a lifecycle state", "-");
        addProcess("GROUP", "Nested work", "ROOT");
        addTask("A", "First task", "GROUP");
        addTask("B", "Second task", "GROUP");
    }
    void buildDependencies() override { addDependency("A", "B", "*"); }
    void buildServices() override { if (service) setService("A", "CheckpointSimulation"); }
    void buildRouting() override {
        setRouting("A", new RoleBasedRouting("Worker"));
        setRouting("B", new RoleBasedRouting("Worker"));
    }
    void buildExtras() override {
        if (deadlineHours) setDeadlineHours("A", deadlineHours);
        if (service) markCheckpoint("A");
        if (decorated) {
            addPriority("A", 2);
            addAudit("A");
            addAudit("GROUP");
            addAudit("ROOT");
        }
    }
};

class RuntimeApprovalBuilder : public WorkflowDefinitionBuilder {
public:
    void buildStructure() override {
        startDefinition("runtime-approval-deadline", "Approval deadlines", 1);
        addProcess("ROOT", "Approval process", "-");
        addTask("APPROVE", "Approve request", "ROOT");
    }
    void buildDependencies() override {}
    void buildServices() override {}
    void buildRouting() override {
        auto officer = new ConcreteApprovalHandler("Officer", new ThresholdRule("amount", 1000), 1);
        auto manager = new ConcreteApprovalHandler("Manager", new ThresholdRule("amount", 10000), 2);
        auto director = new ConcreteApprovalHandler("Director", new AlwaysAcceptRule(), 3);
        officer->setNext(manager)->setNext(director);
        setRouting("APPROVE", new ApprovalRouting(officer));
    }
    void buildExtras() override { addAudit("APPROVE"); }
};

class CheckpointSimulation : public ExternalService {
public:
    int calls = 0;
    bool sawRequest = false;
    std::string name() const override { return "CheckpointSimulation"; }
    ServiceResult invoke(const ServiceRequest &request) override {
        ++calls;
        sawRequest = request.get("request") == "original";
        if (calls == 1) return ServiceResult::failure("Simulated outage");
        ServiceResult result = ServiceResult::ok("Simulated recovery", {{"result", "processed"}});
        result.outcome = "processed";
        return result;
    }
};

class RuntimeAuditRecorder : public AuditLogger {
public:
    std::vector<WorkflowEvent> events;
    void onEvent(const WorkflowEvent &event) override { events.push_back(event); }
};

void addWorkers(TividyWorkflowFacade &facade) {
    facade.addParticipant("worker", "First worker", {"Worker"});
    facade.addParticipant("peer", "Second worker", {"Worker"});
    facade.addParticipant("outsider", "Different role", {"Student"});
}
}

TEST_SUITE("Runtime integration") {
    TEST_CASE("automatic nested process completion and rejection use their audit decorators") {
        RuntimeAuditRecorder audit;
        WorkflowEngine engine;
        Participant *worker = new Participant("audit-worker", "Worker", "Worker");
        engine.getDirectory().addParticipant(worker);
        WorkflowDefinition *definition = new WorkflowDefinition("runtime-process-audit", "Audited process", 1);
        SubProcess *root = new SubProcess("ROOT", "Root");
        SubProcess *group = new SubProcess("GROUP", "Nested process");
        Task *task = new Task("TASK", "Task");
        task->setRouting(new RoleBasedRouting("Worker"));
        group->add(task);
        root->add(new AuditDecorator(group, &audit));
        definition->setRoot(new AuditDecorator(root, &audit));
        REQUIRE(engine.registerDefinition(definition) == "runtime-process-audit");
        WorkflowInstance *instance = engine.createInstance("runtime-process-audit", InstanceData());
        REQUIRE(instance != nullptr);
        REQUIRE(instance->start());
        REQUIRE(engine.executeAction(new StartAction(instance, "TASK", worker)));
        EventType expected = EventType::COMPLETED;
        SUBCASE("completion") {
            REQUIRE(engine.executeAction(new CompleteAction(instance, "TASK", worker, "done")));
            CHECK(instance->status() == "completed");
        }
        SUBCASE("unhandled rejection") {
            expected = EventType::REJECTED;
            REQUIRE(engine.executeAction(new RejectAction(instance, "TASK", worker, "failed review")));
            CHECK(instance->status() == "terminated");
        }
        for (const std::string id : {"ROOT", "GROUP"}) {
            int matches = 0;
            for (const WorkflowEvent &event : audit.events) {
                if (event.itemId == id && event.type == expected) {
                    ++matches;
                    CHECK(event.instanceId == instance->getId());
                }
            }
            CHECK(matches == 1);
        }
    }

    TEST_CASE("normal facade wires decorated runtime trees and undo reopens dependencies") {
        TividyWorkflowFacade facade;
        addWorkers(facade);
        RuntimeLinearBuilder builder("runtime-decorated", true);
        REQUIRE(facade.registerDefinition(builder) == "runtime-decorated");
        InstanceData initial;
        initial.set("request", "original");
        const std::string first = facade.startInstance("runtime-decorated", initial);
        const std::string second = facade.startInstance("runtime-decorated", initial);
        REQUIRE_FALSE(first.empty());
        REQUIRE_FALSE(second.empty());
        REQUIRE(first != second);
        WorkflowInstance *instance = facade.getInstance(first);
        WorkflowInstance *other = facade.getInstance(second);
        REQUIRE(instance != nullptr);
        REQUIRE(other != nullptr);
        REQUIRE(instance->findItem("A") != nullptr);
        REQUIRE(instance->findItem("B") != nullptr);
        Participant *actor = facade.findParticipant("worker");
        CHECK(instance->findItem("A")->getStateName() == "Assigned");
        CHECK(instance->findItem("A")->getAssignee() == actor);
        CHECK(instance->findItem("A")->getInstanceId() == first);
        CHECK(instance->findItem("B")->getStateName() == "Created");
        CHECK(instance->getRoot()->getStateName() == "InProgress");
        CHECK(instance->getDefinitionId() == "runtime-decorated");

        REQUIRE(facade.start(first, "A", actor));
        REQUIRE(facade.complete(first, "A", actor, "done"));
        CHECK(instance->findItem("B")->getStateName() == "Assigned");
        CHECK(other->findItem("A")->getStateName() == "Assigned");
        CHECK(other->findItem("B")->getStateName() == "Created");

        REQUIRE(facade.undoLast());
        CHECK(instance->findItem("A")->getStateName() == "InProgress");
        CHECK(instance->findItem("A")->getAssignee() == actor);
        CHECK(instance->findItem("A")->getOutcome().empty());
        CHECK(instance->findItem("B")->getStateName() == "Created");
        CHECK(instance->findItem("B")->getAssignee() == nullptr);
        REQUIRE(facade.complete(first, "A", actor, "done-again"));
        CHECK(instance->findItem("B")->getStateName() == "Assigned");
        REQUIRE(facade.start(first, "B", actor));
        REQUIRE(facade.complete(first, "B", actor, "done"));
        CHECK(facade.getStatus(first) == "completed");
        REQUIRE(facade.undoLast());
        CHECK(facade.getStatus(first) == "running");
        CHECK(instance->getRoot()->getStateName() == "InProgress");
        CHECK(instance->findItem("B")->getStateName() == "InProgress");
        CHECK(instance->findItem("B")->getAssignee() == actor);
        REQUIRE(facade.complete(first, "B", actor, "done"));
        CHECK(facade.getStatus(first) == "completed");
        CHECK(facade.getStatus(second) == "running");
    }

    TEST_CASE("commands enforce registered identity assignee and eligible delegation") {
        TividyWorkflowFacade facade;
        addWorkers(facade);
        RuntimeLinearBuilder builder("runtime-auth");
        REQUIRE(facade.registerDefinition(builder) == "runtime-auth");
        const std::string id = facade.startInstance("runtime-auth", InstanceData());
        WorkflowInstance *instance = facade.getInstance(id);
        REQUIRE(instance != nullptr);
        Participant *actor = facade.findParticipant("worker");
        Participant *peer = facade.findParticipant("peer");
        Participant *outsider = facade.findParticipant("outsider");
        Participant impostor("worker", "Same ID is not membership", "Worker");
        StartAction forged(instance, "A", &impostor);
        CHECK_FALSE(forged.execute());
        CHECK_FALSE(facade.start(id, "A", outsider));
        CHECK_FALSE(facade.claim(id, "A", peer)); // no stealing assigned work
        CHECK_FALSE(facade.cancel(id, "B", actor)); // no bypassing a dependency
        CHECK_FALSE(facade.delegate(id, "A", actor, outsider));
        CHECK_FALSE(facade.delegate(id, "A", actor, nullptr));
        CHECK(facade.actionHistory().empty());
        CHECK(instance->findItem("A")->getAssignee() == actor);
        REQUIRE(facade.delegate(id, "A", actor, peer));
        CHECK(instance->findItem("A")->getAssignee() == peer);
        CHECK_FALSE(facade.start(id, "A", actor));
        REQUIRE(facade.start(id, "A", peer));
        CompleteAction outsiderComplete(instance, "A", outsider, "stolen");
        CHECK_FALSE(outsiderComplete.execute());
        CHECK_FALSE(facade.reject(id, "A", actor, "stolen"));
        CHECK_FALSE(facade.cancel(id, "A", outsider));
        REQUIRE(facade.complete(id, "A", peer, "done"));
        CHECK(instance->findItem("B")->getStateName() == "Assigned");
    }

    TEST_CASE("role deadline expiry is deterministic undoable and reclaim checks eligibility") {
        TividyWorkflowFacade facade;
        addWorkers(facade);
        RuntimeLinearBuilder builder("runtime-role-deadline", true, 1);
        REQUIRE(facade.registerDefinition(builder) == "runtime-role-deadline");
        const std::string id = facade.startInstance("runtime-role-deadline", InstanceData());
        WorkflowInstance *instance = facade.getInstance(id);
        REQUIRE(instance != nullptr);
        WorkItem *item = instance->findItem("A");
        REQUIRE(item != nullptr);
        const int initialLevel = item->getEscalationLevel();
        const std::time_t due = instance->getDeadline("A");
        REQUIRE(due != 0);
        facade.checkDeadlines(due - 1);
        CHECK(item->getStateName() == "Assigned");
        CHECK(facade.actionHistory().empty());
        facade.checkDeadlines(due);
        CHECK(item->getStateName() == "Escalated");
        CHECK(item->getAssignee() == nullptr);
        CHECK(item->getEscalationLevel() == initialLevel + 1);
        CHECK(instance->getDeadline("A") == 0);
        REQUIRE(facade.undoLast());
        CHECK(item->getStateName() == "Assigned");
        CHECK(item->getAssignee() == facade.findParticipant("worker"));
        CHECK(item->getEscalationLevel() == initialLevel);
        CHECK(instance->getDeadline("A") == due);
        facade.checkDeadlines(due);
        CHECK(item->getStateName() == "Escalated");
        CHECK_FALSE(facade.claim(id, "A", facade.findParticipant("outsider")));
        REQUIRE(facade.claim(id, "A", facade.findParticipant("peer")));
        CHECK(item->getAssignee() == facade.findParticipant("peer"));
        CHECK(instance->getDeadline("A") != 0);
    }

    TEST_CASE("approval deadline advances from the actual accepting authority and undo restores it") {
        TividyWorkflowFacade facade;
        facade.addParticipant("officer", "Officer", {"Officer"});
        facade.addParticipant("manager", "Manager", {"Manager"});
        facade.addParticipant("director", "Director", {"Director"});
        RuntimeApprovalBuilder builder;
        REQUIRE(facade.registerDefinition(builder) == "runtime-approval-deadline");
        InstanceData initial;
        initial.set("amount", "2000");
        const std::string id = facade.startInstance("runtime-approval-deadline", initial);
        WorkflowInstance *instance = facade.getInstance(id);
        REQUIRE(instance != nullptr);
        WorkItem *item = instance->findItem("APPROVE");
        REQUIRE(item != nullptr);
        CHECK(item->getAssignee() == facade.findParticipant("manager"));
        CHECK(item->getEscalationLevel() == 1);
        CHECK_FALSE(facade.delegate(id, "APPROVE", facade.findParticipant("manager"),
                                    facade.findParticipant("director")));
        const std::time_t due = instance->getDeadline("APPROVE");
        REQUIRE(due != 0);
        facade.checkDeadlines(due);
        CHECK(item->getAssignee() == facade.findParticipant("director"));
        CHECK(item->getStateName() == "Assigned");
        CHECK(item->getEscalationLevel() == 2);
        CHECK(instance->getDeadline("APPROVE") == due + 3 * 3600);
        REQUIRE(facade.undoLast());
        CHECK(item->getAssignee() == facade.findParticipant("manager"));
        CHECK(item->getEscalationLevel() == 1);
        CHECK(instance->getDeadline("APPROVE") == due);
        facade.checkDeadlines(due);
        CHECK(item->getAssignee() == facade.findParticipant("director"));
        CHECK_FALSE(facade.start(id, "APPROVE", facade.findParticipant("manager")));
        REQUIRE(facade.start(id, "APPROVE", facade.findParticipant("director")));
        REQUIRE(facade.complete(id, "APPROVE", facade.findParticipant("director"), "approved"));
        CHECK(facade.getStatus(id) == "completed");
    }

    TEST_CASE("maximum supplied deadline timestamp cannot overflow a renewed deadline") {
        TividyWorkflowFacade facade;
        facade.addParticipant("officer", "Officer", {"Officer"});
        facade.addParticipant("manager", "Manager", {"Manager"});
        facade.addParticipant("director", "Director", {"Director"});
        RuntimeApprovalBuilder builder;
        REQUIRE_FALSE(facade.registerDefinition(builder).empty());
        InstanceData initial;
        initial.set("amount", "2000");
        const std::string id = facade.startInstance("runtime-approval-deadline", initial);
        WorkflowInstance *instance = facade.getInstance(id);
        REQUIRE(instance != nullptr);
        REQUIRE(instance->getDeadline("APPROVE") != 0);
        facade.checkDeadlines(std::numeric_limits<std::time_t>::max());
        CHECK(instance->findItem("APPROVE")->getAssignee() == facade.findParticipant("director"));
        CHECK(instance->getDeadline("APPROVE") == 0);
        REQUIRE(facade.undoLast());
        CHECK(instance->findItem("APPROVE")->getAssignee() == facade.findParticipant("manager"));
        CHECK(instance->getDeadline("APPROVE") != 0);
    }

    TEST_CASE("failed checkpoint service can undo and retry without corrupting downstream state") {
        TividyWorkflowFacade facade;
        addWorkers(facade);
        auto service = new CheckpointSimulation();
        facade.addService(service);
        RuntimeLinearBuilder builder("runtime-checkpoint", true, 0, true);
        REQUIRE(facade.registerDefinition(builder) == "runtime-checkpoint");
        InstanceData initial;
        initial.set("request", "original");
        const std::string id = facade.startInstance("runtime-checkpoint", initial);
        WorkflowInstance *instance = facade.getInstance(id);
        REQUIRE(instance != nullptr);
        Participant *actor = facade.findParticipant("worker");
        REQUIRE(facade.start(id, "A", actor));
        REQUIRE(facade.complete(id, "A", actor, "ignored"));
        CHECK(service->calls == 1);
        CHECK(service->sawRequest);
        CHECK(instance->findItem("A")->getStateName() == "Escalated");
        CHECK(instance->findItem("B")->getStateName() == "Created");
        CHECK(instance->getData().get("service.A.success") == "false");
        REQUIRE_FALSE(facade.actionHistory().empty());
        CHECK(facade.actionHistory().back().find("Checkpoint before A") != std::string::npos);
        REQUIRE(facade.undoLast());
        CHECK(instance->findItem("A")->getStateName() == "InProgress");
        CHECK(instance->findItem("A")->getAssignee() == actor);
        CHECK_FALSE(instance->getData().has("service.A.success"));
        CHECK(instance->getData().get("request") == "original");
        CHECK(instance->findItem("B")->getStateName() == "Created");
        REQUIRE(facade.complete(id, "A", actor, "ignored"));
        CHECK(service->calls == 2);
        CHECK(instance->getData().get("result") == "processed");
        CHECK(instance->findItem("A")->getOutcome() == "processed");
        CHECK(instance->findItem("B")->getStateName() == "Assigned");
        CHECK_FALSE(facade.complete(id, "A", actor, "repeat"));
        CHECK(service->calls == 2);
    }

    TEST_CASE("invalid definitions and repeated registration cannot leave a dangling owned tree") {
        WorkflowEngine engine;
        WorkflowDefinition *definition = new WorkflowDefinition("runtime-owned", "Owned", 1);
        definition->setRoot(new SubProcess("ROOT", "Root"));
        REQUIRE(engine.registerDefinition(definition) == "runtime-owned");
        CHECK(engine.registerDefinition(definition).empty());
        WorkflowInstance *instance = engine.createInstance("runtime-owned", InstanceData());
        REQUIRE(instance != nullptr);
        CHECK(instance->status() == "created");
        REQUIRE(instance->start());
        CHECK(instance->status() == "completed");
        CHECK_FALSE(instance->start());
        WorkflowDefinition *invalid = new WorkflowDefinition("runtime-invalid", "No root", 1);
        CHECK(engine.registerDefinition(invalid).empty());
        CHECK(engine.createInstance("runtime-invalid", InstanceData()) == nullptr);
        CHECK(engine.definitionIds().size() == 1);
        CHECK(engine.instanceIds().size() == 1);
    }
}
