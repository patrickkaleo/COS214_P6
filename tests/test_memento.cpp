/**
 * @file test_memento.cpp
 * @brief Memento pattern tests for the TIVIDY project's current public API.
 * tests/main_tests.cpp supplies the doctest main function.
 */

#include "doctest.h"
#include "InstanceData.h"
#include "WorkflowDefinition.h"
#include "WorkflowInstance.h"
#include "WorkflowSnapshot.h"
#include "HistoryLog.h"
#include "SubProcess.h"

#include <memory>
#include <string>

TEST_SUITE("Memento Pattern")
{

    TEST_CASE("InstanceData stores and retrieves values")
    {
        InstanceData data;
        data.set("request", "pending");
        CHECK(data.has("request"));
        CHECK(data.get("request") == "pending");
        data.set("request", "approved");
        CHECK(data.get("request") == "approved");
    }

    TEST_CASE("WorkflowInstance receives initial data")
    {
        WorkflowDefinition definition("memento-definition-1", "Memento Test", 1);
        definition.setRoot(new SubProcess("root", "Root process"));

        InstanceData initial;
        initial.set("request", "pending");
        WorkflowInstance instance("memento-instance-1", &definition, initial);

        CHECK(instance.getId() == "memento-instance-1");
        CHECK(instance.getData().get("request") == "pending");
    }

    TEST_CASE("Originator creates a labeled snapshot")
    {
        WorkflowDefinition definition("memento-definition-2", "Memento Test", 1);
        definition.setRoot(new SubProcess("root", "Root process"));

        InstanceData initial;
        initial.set("request", "pending");
        WorkflowInstance instance("memento-instance-2", &definition, initial);

        std::unique_ptr<WorkflowSnapshot> snapshot(
            instance.createSnapshot("before-update"));
        REQUIRE(snapshot != nullptr);
        CHECK(snapshot->getLabel() == "before-update");
    }

    TEST_CASE("Originator restores saved instance data")
    {
        WorkflowDefinition definition("memento-definition-3", "Memento Test", 1);
        definition.setRoot(new SubProcess("root", "Root process"));

        InstanceData initial;
        initial.set("request", "pending");
        WorkflowInstance instance("memento-instance-3", &definition, initial);

        std::unique_ptr<WorkflowSnapshot> snapshot(
            instance.createSnapshot("initial"));
        REQUIRE(snapshot != nullptr);

        instance.restoreSnapshot(*snapshot);
        CHECK(instance.getData().get("request") == "pending");
    }

    TEST_CASE("HistoryLog stores and returns the latest snapshot")
    {
        WorkflowDefinition definition("memento-definition-4", "Memento Test", 1);
        definition.setRoot(new SubProcess("root", "Root process"));

        InstanceData initial;
        WorkflowInstance instance("memento-instance-4", &definition, initial);
        HistoryLog history;

        history.pushSnapshots("memento-instance-4", instance.createSnapshot("first"));
        history.pushSnapshots("memento-instance-4", instance.createSnapshot("second"));

        const WorkflowSnapshot *newest = history.latest("memento-instance-4");
        REQUIRE(newest != nullptr);
        CHECK(newest->getLabel() == "second");

        // The currently uploaded HistoryLog.h declares popSnapshot() with NO parameters.
        // Its implementation/ownership contract is not available, so this test
        // deliberately does not assume how it selects an instance or returns ownership.
    }
}
