/**
 * @file test_memento.cpp
 * @brief doctest tests for the TIVIDY Memento pattern.
 *
 * This file intentionally has NO main(): tests/main_tests.cpp supplies it.
 * These are integration-facing tests of the real WorkflowInstance, WorkflowSnapshot,
 * and HistoryLog. Build with the project's real implementations.
 */

#include "doctest.h"
#include "WorkflowInstance.h"
#include "WorkflowSnapshot.h"
#include "HistoryLog.h"
#include "SubProcess.h"
#include "Task.h"
#include "DependencyMediator.h"
#include "AssignedState.h"
#include "CompletedState.h"

#include <memory>
#include <string>

TEST_SUITE("Memento Pattern") {

    TEST_CASE("HistoryLog returns null for an instance with no checkpoints") {
        HistoryLog history;
        CHECK(history.latest("missing-instance") == nullptr);
        CHECK(history.popSnapshots("missing-instance") == nullptr);
    }

    TEST_CASE("Originator creates a labeled snapshot and restores instance data") {
        InstanceData initialData;
        initialData.set("request", "pending");

        // The instance takes ownership of the root and mediator.
        WorkflowInstance instance(
            "instance-1", nullptr,
            new SubProcess("root", "Main workflow"),
            new DependencyMediator(), initialData, nullptr);

        std::unique_ptr<WorkflowSnapshot> snapshot(
            instance.createSnapshot("before-update"));
        REQUIRE(snapshot != nullptr);
        CHECK(snapshot->getLabel() == "before-update");

        instance.getData().set("request", "processed");
        REQUIRE(instance.getData().get("request") == "processed");

        instance.restoreSnapshot(*snapshot);
        CHECK(instance.getData().get("request") == "pending");
    }

    TEST_CASE("Originator restores task lifecycle state and outcome") {
        SubProcess* root = new SubProcess("root", "Main workflow");
        Task* review = new Task("review", "Review request");
        root->add(review); // root owns the task

        InstanceData initialData;
        WorkflowInstance instance(
            "instance-2", nullptr, root,
            new DependencyMediator(), initialData, nullptr);

        // Set a known state directly to isolate Memento from routing and events.
        review->setState(new AssignedState());
        review->setOutcome("pending");
        REQUIRE(review->getStateName() == "Assigned");

        std::unique_ptr<WorkflowSnapshot> snapshot(
            instance.createSnapshot("before-completion"));
        REQUIRE(snapshot != nullptr);

        review->setState(new CompletedState());
        review->setOutcome("approved");
        REQUIRE(review->getStateName() == "Completed");

        instance.restoreSnapshot(*snapshot);
        CHECK(review->getStateName() == "Assigned");
        CHECK(review->getOutcome() == "pending");
    }

    TEST_CASE("HistoryLog returns the newest checkpoint first (LIFO)") {
        InstanceData data;
        WorkflowInstance instance(
            "instance-3", nullptr,
            new SubProcess("root", "Main workflow"),
            new DependencyMediator(), data, nullptr);
        HistoryLog history;

        // pushSnapshots transfers ownership to HistoryLog.
        history.pushSnapshots("instance-3", instance.createSnapshot("first"));
        history.pushSnapshots("instance-3", instance.createSnapshot("second"));

        REQUIRE(history.latest("instance-3") != nullptr);
        CHECK(history.latest("instance-3")->getLabel() == "second");

        // popSnapshots transfers ownership back to the caller.
        std::unique_ptr<WorkflowSnapshot> second(
            history.popSnapshots("instance-3"));
        REQUIRE(second != nullptr);
        CHECK(second->getLabel() == "second");

        REQUIRE(history.latest("instance-3") != nullptr);
        CHECK(history.latest("instance-3")->getLabel() == "first");

        std::unique_ptr<WorkflowSnapshot> first(
            history.popSnapshots("instance-3"));
        REQUIRE(first != nullptr);
        CHECK(first->getLabel() == "first");
        CHECK(history.latest("instance-3") == nullptr);
    }

    TEST_CASE("HistoryLog keeps checkpoint histories separate for each instance") {
        InstanceData data;
        WorkflowInstance a(
            "A", nullptr, new SubProcess("root-a", "Workflow A"),
            new DependencyMediator(), data, nullptr);
        WorkflowInstance b(
            "B", nullptr, new SubProcess("root-b", "Workflow B"),
            new DependencyMediator(), data, nullptr);

        HistoryLog history;
        history.pushSnapshots("A", a.createSnapshot("checkpoint-A"));
        history.pushSnapshots("B", b.createSnapshot("checkpoint-B"));

        REQUIRE(history.latest("A") != nullptr);
        REQUIRE(history.latest("B") != nullptr);
        CHECK(history.latest("A")->getLabel() == "checkpoint-A");
        CHECK(history.latest("B")->getLabel() == "checkpoint-B");

        std::unique_ptr<WorkflowSnapshot> removed(history.popSnapshots("A"));
        REQUIRE(removed != nullptr);
        CHECK(history.latest("A") == nullptr);
        REQUIRE(history.latest("B") != nullptr);
        CHECK(history.latest("B")->getLabel() == "checkpoint-B");
    }
}
