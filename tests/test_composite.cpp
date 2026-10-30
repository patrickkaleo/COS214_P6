/**
 * @file test_composite.cpp
 * @brief Unit tests for the Composite hierarchy (WorkItem, Task, SubProcess) and Iterator.
 */

#include "doctest.h"

#include <string>
#include <vector>

#include "WorkItem.h"
#include "Task.h"
#include "SubProcess.h"
#include "WorkItemIterator.h"
#include "DepthFirstWorkItemIterator.h"
#include "Participant.h"
#include "InstanceData.h"
#include "ParticipantDirectory.h"
#include "RoleBasedRouting.h"

TEST_SUITE("Composite pattern - WorkItem / Task / SubProcess") {

    TEST_CASE("Task: identity and default configuration") {
        Task task("T1", "Capture Application");
        CHECK(task.getId() == "T1");
        CHECK(task.getName() == "Capture Application");
        CHECK(task.getOutcome().empty());
        CHECK(task.getAssignee() == nullptr);
        CHECK(task.isCheckpoint() == false);
        CHECK(task.getServiceName().empty());
        CHECK(task.getRouting() == nullptr);
        CHECK(task.getParent() == nullptr);
        CHECK(task.getMediator() == nullptr);
    }

    TEST_CASE("SubProcess: add children and ownership structure") {
        SubProcess* root = new SubProcess("INTAKE", "Application Intake");
        Task* capture = new Task("CAPTURE", "Capture Application");
        Task* verify = new Task("VERIFY", "Verify Application");

        root->add(capture);
        root->add(verify);

        CHECK(root->childCount() == 2);
        CHECK(root->getChild(0) == capture);
        CHECK(root->getChild(1) == verify);
        CHECK(capture->getParent() == root);
        CHECK(verify->getParent() == root);
        CHECK(root->getChildren().size() == 2);

        // Out of range
        CHECK(root->getChild(-1) == nullptr);
        CHECK(root->getChild(99) == nullptr);

        delete root; // deletes children
    }

    TEST_CASE("SubProcess: remove child by id") {
        SubProcess root("ROOT", "Root");
        root.add(new Task("A", "Task A"));
        root.add(new Task("B", "Task B"));
        CHECK(root.childCount() == 2);

        root.remove("A");
        CHECK(root.childCount() == 1);
        REQUIRE(root.getChild(0) != nullptr);
        CHECK(root.getChild(0)->getId() == "B");

        root.remove("missing");
        CHECK(root.childCount() == 1);
    }

    TEST_CASE("Task: service name and checkpoint flags") {
        Task task("PAY", "Bursary Payment");
        task.setServiceName("BankGateway");
        task.setCheckpoint(true);

        CHECK(task.getServiceName() == "BankGateway");
        CHECK(task.isCheckpoint() == true);
    }

    TEST_CASE("Task: routing strategy attach and clone copies strategy") {
        Task original("CAP", "Capture");
        original.setServiceName("DocVerify");
        original.setCheckpoint(true);
        original.setRouting(new RoleBasedRouting("Admissions Officer"));

        REQUIRE(original.getRouting() != nullptr);

        WorkItem* clonedItem = original.clone();
        REQUIRE(clonedItem != nullptr);
        Task* cloned = dynamic_cast<Task*>(clonedItem);
        REQUIRE(cloned != nullptr);

        CHECK(cloned->getId() == "CAP");
        CHECK(cloned->getName() == "Capture");
        CHECK(cloned->getServiceName() == "DocVerify");
        CHECK(cloned->isCheckpoint() == true);
        CHECK(cloned->getRouting() != nullptr);
        // Independent strategy object
        CHECK(cloned->getRouting() != original.getRouting());

        delete clonedItem;
    }

    TEST_CASE("SubProcess: clone deep-copies the tree") {
        SubProcess* root = new SubProcess("INTAKE", "Intake");
        root->add(new Task("CAPTURE", "Capture"));
        root->add(new Task("VERIFY", "Verify"));

        WorkItem* clonedItem = root->clone();
        REQUIRE(clonedItem != nullptr);
        SubProcess* cloned = dynamic_cast<SubProcess*>(clonedItem);
        REQUIRE(cloned != nullptr);

        CHECK(cloned->getId() == "INTAKE");
        CHECK(cloned->childCount() == 2);
        CHECK(cloned->getChild(0)->getId() == "CAPTURE");
        CHECK(cloned->getChild(1)->getId() == "VERIFY");
        // Different objects
        CHECK(cloned->getChild(0) != root->getChild(0));
        CHECK(cloned->getChild(0)->getParent() == cloned);

        delete clonedItem;
        delete root;
    }

    TEST_CASE("Nested SubProcess: composite of composites") {
        SubProcess* root = new SubProcess("ROOT", "Root Process");
        SubProcess* branch = new SubProcess("BRANCH", "Parallel Branch");
        branch->add(new Task("T1", "Task One"));
        branch->add(new Task("T2", "Task Two"));
        root->add(branch);
        root->add(new Task("T3", "Task Three"));

        CHECK(root->childCount() == 2);
        SubProcess* nested = dynamic_cast<SubProcess*>(root->getChild(0));
        REQUIRE(nested != nullptr);
        CHECK(nested->childCount() == 2);
        CHECK(root->getChild(1)->getId() == "T3");

        delete root;
    }

    TEST_CASE("Iterator: depth-first traversal visits root then children") {
        SubProcess* root = new SubProcess("ROOT", "Root");
        root->add(new Task("A", "Task A"));
        root->add(new Task("B", "Task B"));

        WorkItemIterator* it = root->createIterator();
        REQUIRE(it != nullptr);

        std::vector<std::string> order;
        for (it->first(); !it->isDone(); it->next()) {
            WorkItem* current = it->current();
            REQUIRE(current != nullptr);
            order.push_back(current->getId());
        }

        REQUIRE(order.size() == 3);
        CHECK(order[0] == "ROOT");
        CHECK(order[1] == "A");
        CHECK(order[2] == "B");

        delete it;
        delete root;
    }

    TEST_CASE("Iterator: nested tree depth-first order") {
        SubProcess* root = new SubProcess("R", "Root");
        SubProcess* mid = new SubProcess("M", "Mid");
        mid->add(new Task("T1", "One"));
        mid->add(new Task("T2", "Two"));
        root->add(mid);
        root->add(new Task("T3", "Three"));

        WorkItemIterator* it = root->createIterator();
        std::vector<std::string> order;
        for (it->first(); !it->isDone(); it->next()) {
            order.push_back(it->current()->getId());
        }

        // Pre-order: R, M, T1, T2, T3
        REQUIRE(order.size() == 5);
        CHECK(order[0] == "R");
        CHECK(order[1] == "M");
        CHECK(order[2] == "T1");
        CHECK(order[3] == "T2");
        CHECK(order[4] == "T3");

        delete it;
        delete root;
    }

    TEST_CASE("WorkItem: parent and mediator setters") {
        SubProcess root("R", "Root");
        Task task("T", "Leaf");
        task.setParent(&root);
        CHECK(task.getParent() == &root);

        task.setMediator(nullptr);
        CHECK(task.getMediator() == nullptr);
    }

    TEST_CASE("Task: setRoutingContext does not crash with RoleBasedRouting") {
        ParticipantDirectory directory;
        directory.addParticipant(new Participant("p1", "Alice", "Admissions Officer"));

        InstanceData data;
        Task task("CAP", "Capture");
        task.setRouting(new RoleBasedRouting("Admissions Officer"));
        task.setRoutingContext(&data, &directory);

        // onAvailable is invoked after a successful makeAvailable transition.
        // Without a full state machine accepting makeAvailable, we only verify that routing context can be set and routing is present
        CHECK(task.getRouting() != nullptr);
    }
}
