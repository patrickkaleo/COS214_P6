#include "doctest.h"

#include "AuditDecorator.h"
#include "AuditLogger.h"
#include "DependencyMediator.h"
#include "DepthFirstWorkItemIterator.h"
#include "InProgressState.h"
#include "Participant.h"
#include "ParticipantDirectory.h"
#include "PriorityDecorator.h"
#include "RoleBasedRouting.h"
#include "SubProcess.h"
#include "Task.h"
#include "WorkflowEventListener.h"

#include <functional>
#include <memory>
#include <vector>

namespace {
class ItemCallback : public WorkflowEventListener {
public:
    explicit ItemCallback(const std::function<void(const WorkflowEvent&)>& callback)
        : callback_(callback) {}
    void onEvent(const WorkflowEvent& event) override { callback_(event); }
private:
    std::function<void(const WorkflowEvent&)> callback_;
};
}

TEST_SUITE("Decorator transparency and per-item observers") {
    TEST_CASE("Outer WorkItem exposes and restores the inner task lifecycle") {
        Participant first("first", "First reviewer"), second("second", "Second reviewer");
        AuditLogger audit;
        Task* task = new Task("review", "Review");
        std::unique_ptr<WorkItem> item(new AuditDecorator(new PriorityDecorator(task, 2), &audit));
        std::vector<WorkflowEvent> events;
        ItemCallback listener([&](const WorkflowEvent& event) { events.push_back(event); });
        item->attach(&listener);
        item->setInstanceId("decorated-instance");
        item->setEscalationLevel(2);
        item->incrementEscalation();
        CHECK(task->getInstanceId() == "decorated-instance");
        CHECK(task->getEscalationLevel() == 3);
        CHECK(item->getEscalationLevel() == 3);

        CHECK_FALSE(item->complete("too early"));
        CHECK(audit.getEntryCount() == 0);
        REQUIRE(item->makeAvailable());
        CHECK(item->getStateName() == "Available");
        REQUIRE(item->assign(&first));
        CHECK(item->getAssignee() == &first);
        REQUIRE(item->start());
        REQUIRE(item->complete("approved"));
        CHECK(item->isFinished());
        CHECK(item->getStateName() == "Completed");
        CHECK(item->getState() == task->getState());
        CHECK(item->getOutcome() == "approved");
        REQUIRE(audit.getEntryCount() == 4);
        CHECK(audit.getAuditTrail().back().find("INSTANCE:decorated-instance") != std::string::npos);
        CHECK_FALSE(item->complete("again"));
        CHECK(audit.getEntryCount() == 4);
        REQUIRE(events.size() == 4);
        CHECK(events.back().instanceId == "decorated-instance");
        CHECK(events.back().outcome == "approved");

        // Memento restoration uses these setters through the outer WorkItem pointer.
        item->setState(new InProgressState());
        item->setOutcome("");
        item->setAssignee(&second);
        CHECK_FALSE(item->isFinished());
        CHECK(task->getAssignee() == &second);
        CHECK(task->getOutcome().empty());
        REQUIRE(item->reject("needs changes"));
        CHECK(item->getStateName() == "Rejected");
        CHECK(item->getOutcome() == "needs changes");
        CHECK(item->isFinished());
        item->detach(&listener);
    }

    TEST_CASE("Decorated root and nested process preserve routing traversal and dependencies") {
        ParticipantDirectory directory;
        std::shared_ptr<Participant> reviewer(new Participant("reviewer", "Reviewer", "Review"));
        directory.addParticipant(reviewer);
        InstanceData data;
        AuditLogger audit;
        SubProcess* rootProcess = new SubProcess("root", "Root");
        SubProcess* nested = new SubProcess("nested", "Nested branch");
        Task* a = new Task("a", "First task");
        Task* b = new Task("b", "Second task");
        a->setRouting(new RoleBasedRouting("Review"));
        b->setRouting(new RoleBasedRouting("Review"));
        WorkItem* decoratedA = new AuditDecorator(new PriorityDecorator(a, 1), &audit);
        WorkItem* decoratedB = new PriorityDecorator(b, 1);
        nested->add(decoratedA);
        nested->add(decoratedB);
        WorkItem* decoratedNested = new AuditDecorator(new PriorityDecorator(nested, 1), &audit);
        rootProcess->add(decoratedNested);
        std::unique_ptr<WorkItem> root(new PriorityDecorator(rootProcess, 1));
        DependencyMediator mediator(std::vector<DependencyRule>{DependencyRule("a", "b", "approved")});
        mediator.registerTree(*root);
        root->setRoutingContext(&data, &directory);

        CHECK(decoratedNested->getParent() == rootProcess);
        CHECK(nested->getParent() == rootProcess);
        CHECK(a->getParent() == nested);
        CHECK(decoratedA->getMediator() == &mediator);
        CHECK(a->getMediator() == &mediator);
        CHECK(root->childCount() == 1);
        CHECK(decoratedNested->childCount() == 2);
        CHECK(rootProcess->find("a") == decoratedA);
        int visited = 0;
        for (DepthFirstWorkItemIterator it(root.get()); !it.isDone(); it.next()) ++visited;
        CHECK(visited == 4);

        REQUIRE(root->makeAvailable());
        CHECK(root->getStateName() == "InProgress");
        CHECK(decoratedA->getStateName() == "Assigned");
        CHECK(decoratedA->getAssignee() == reviewer.get());
        CHECK(decoratedB->getStateName() == "Created");
        REQUIRE(decoratedA->start());
        REQUIRE(decoratedA->complete("approved"));
        CHECK(decoratedB->getStateName() == "Assigned");
        CHECK(decoratedB->getAssignee() == reviewer.get());
        CHECK_FALSE(root->isFinished());
        REQUIRE(decoratedB->start());
        REQUIRE(decoratedB->complete("done"));
        CHECK(decoratedNested->getStateName() == "Completed");
        CHECK(root->getStateName() == "Completed");
        CHECK(rootProcess->allChildrenFinished());
    }

    TEST_CASE("Cloned decorated trees keep separate tasks and runtime wiring") {
        AuditLogger audit;
        SubProcess* prototypeProcess = new SubProcess("root", "Root");
        prototypeProcess->add(new AuditDecorator(new PriorityDecorator(new Task("a", "Task"), 4), &audit));
        std::unique_ptr<WorkItem> prototype(new PriorityDecorator(prototypeProcess, 2));
        std::unique_ptr<WorkItem> first(prototype->clone()), second(prototype->clone());
        REQUIRE(first->childCount() == 1);
        REQUIRE(second->childCount() == 1);
        WorkItem* a = first->getChild(0);
        WorkItem* b = second->getChild(0);
        CHECK(a != b);
        CHECK(a->getParent() != b->getParent());
        a->setInstanceId("one");
        b->setInstanceId("two");
        a->setState(new InProgressState());
        REQUIRE(a->complete("done"));
        CHECK(a->getInstanceId() == "one");
        CHECK(b->getInstanceId() == "two");
        CHECK(b->getStateName() == "Created");
        CHECK(b->getOutcome().empty());
        CHECK(prototype->getChild(0)->getStateName() == "Created");
    }

    TEST_CASE("Per-item dispatch survives additions and deletion of a detached pending listener") {
        Task task("task", "Task");
        int deletedListenerCalls = 0, addedCalls = 0, firstCalls = 0;
        std::unique_ptr<ItemCallback> pending(new ItemCallback([&](const WorkflowEvent&) { ++deletedListenerCalls; }));
        std::vector<std::unique_ptr<ItemCallback> > additions;
        ItemCallback first([&](const WorkflowEvent&) {
            if (++firstCalls != 1) return;
            task.detach(pending.get());
            pending.reset();
            for (int i = 0; i < 64; ++i) {
                additions.emplace_back(new ItemCallback([&](const WorkflowEvent&) { ++addedCalls; }));
                task.attach(additions.back().get());
            }
        });
        task.attach(&first);
        task.attach(pending.get());
        WorkflowEvent event(EventType::AVAILABLE, "instance", "task", "Task", "");
        task.notify(event);
        CHECK(firstCalls == 1);
        CHECK(deletedListenerCalls == 0);
        CHECK(addedCalls == 0);
        task.notify(event);
        CHECK(firstCalls == 2);
        CHECK(addedCalls == 64);
        task.detach(&first);
        for (const auto& addition : additions) task.detach(addition.get());
    }

    TEST_CASE("Per-item detach and reattach does not revive the old dispatch registration") {
        Task task("task", "Task");
        int laterCalls = 0;
        bool changed = false;
        ItemCallback later([&](const WorkflowEvent&) { ++laterCalls; });
        ItemCallback first([&](const WorkflowEvent&) {
            if (changed) return;
            changed = true;
            task.detach(&later);
            task.attach(&later);
        });
        task.attach(&first);
        task.attach(&later);
        WorkflowEvent event(EventType::AVAILABLE, "instance", "task", "Task", "");
        task.notify(event);
        CHECK(laterCalls == 0);
        task.notify(event);
        CHECK(laterCalls == 1);
        task.detach(&first);
        task.detach(&later);
    }

    TEST_CASE("Per-item listener can detach itself and dispatch recursively") {
        Task task("task", "Task");
        int selfCalls = 0, laterCalls = 0;
        ItemCallback* selfPointer = nullptr;
        ItemCallback later([&](const WorkflowEvent&) { ++laterCalls; });
        ItemCallback self([&](const WorkflowEvent& event) {
            ++selfCalls;
            task.detach(selfPointer);
            task.notify(event);
        });
        selfPointer = &self;
        task.attach(&self);
        task.attach(&later);
        task.notify(WorkflowEvent(EventType::AVAILABLE, "instance", "task", "Task", ""));
        CHECK(selfCalls == 1);
        CHECK(laterCalls == 2);
        task.detach(&later);
    }
}
