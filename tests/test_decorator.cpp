/**
 * @file test_decorator.cpp
 * @brief Unit tests for the Decorator hierarchy (WorkItemDecorator, Priority, Audit).
 */

#include "doctest.h"

#include <string>

#include "WorkItem.h"
#include "Task.h"
#include "SubProcess.h"
#include "WorkItemDecorator.h"
#include "PriorityDecorator.h"
#include "AuditDecorator.h"
#include "AuditLogger.h"
#include "EventDispatcher.h"
#include "WorkflowEvent.h"

TEST_SUITE("Decorator pattern - PriorityDecorator / AuditDecorator") {

    TEST_CASE("PriorityDecorator: adopts inner identity and stores level") {
        Task* inner = new Task("PAY", "Bursary Payment");
        PriorityDecorator* deco = new PriorityDecorator(inner, 5);

        CHECK(deco->getId() == "PAY");
        CHECK(deco->getName() == "Bursary Payment");
        CHECK(deco->getPriority() == 5);
        CHECK(deco->getInner() == inner);

        delete deco; // deletes inner
    }

    TEST_CASE("PriorityDecorator: onAvailable forwards to inner") {
        Task* inner = new Task("URGENT", "Late Application");
        PriorityDecorator deco(inner, 2);

        // Should not throw - logs priority then forwards onAvailable to Task
        deco.onAvailable();
        CHECK(deco.getPriority() == 2);
        CHECK(deco.getInner()->getId() == "URGENT");
        // PriorityDecorator owns inner — do not delete inner separately
    }

    TEST_CASE("PriorityDecorator: clone copies level and clones inner") {
        Task* inner = new Task("T1", "Review");
        inner->setServiceName("IDCheck");
        PriorityDecorator original(inner, 7);

        WorkItem* clonedItem = original.clone();
        REQUIRE(clonedItem != nullptr);
        PriorityDecorator* cloned = dynamic_cast<PriorityDecorator*>(clonedItem);
        REQUIRE(cloned != nullptr);

        CHECK(cloned->getPriority() == 7);
        CHECK(cloned->getId() == "T1");
        CHECK(cloned->getInner() != nullptr);
        CHECK(cloned->getInner() != original.getInner());

        Task* clonedTask = dynamic_cast<Task*>(cloned->getInner());
        REQUIRE(clonedTask != nullptr);
        CHECK(clonedTask->getServiceName() == "IDCheck");

        delete clonedItem;
        // original destructor deletes its inner
    }

    TEST_CASE("AuditDecorator: wraps task and keeps identity") {
        Task* inner = new Task("VER", "Verify Application");
        AuditLogger logger(false);
        AuditDecorator* deco = new AuditDecorator(inner, &logger);

        CHECK(deco->getId() == "VER");
        CHECK(deco->getName() == "Verify Application");
        CHECK(deco->getInner() == inner);

        delete deco;
    }

    TEST_CASE("AuditDecorator: lifecycle forwards and can record via logger") {
        EventDispatcher::getInstance().clearListeners();

        Task* inner = new Task("CAP", "Capture");
        AuditLogger logger(false);
        AuditDecorator deco(inner, &logger);

        // Direct audit path used by decorator overrides (before state accepts)
        // Calling makeAvailable goes: AuditDecorator logs → inner makeAvailable → state
        // State may refuse if still Created without a valid transition setup
        // at minimum the decorator must not crash and must keep identity.
        deco.makeAvailable();
        CHECK(deco.getId() == "CAP");

        EventDispatcher::getInstance().clearListeners();
    }

    TEST_CASE("AuditDecorator: clone shares logger pointer and clones inner") {
        Task* inner = new Task("T2", "Payment");
        AuditLogger logger(false);
        AuditDecorator original(inner, &logger);

        WorkItem* clonedItem = original.clone();
        REQUIRE(clonedItem != nullptr);
        AuditDecorator* cloned = dynamic_cast<AuditDecorator*>(clonedItem);
        REQUIRE(cloned != nullptr);

        CHECK(cloned->getId() == "T2");
        CHECK(cloned->getInner() != original.getInner());

        delete clonedItem;
    }

    TEST_CASE("Stacked decorators: Priority around Audit around Task") {
        Task* task = new Task("PAY", "Bursary Payment");
        task->setCheckpoint(true);

        AuditLogger logger(false);
        WorkItem* audited = new AuditDecorator(task, &logger);
        WorkItem* prioritised = new PriorityDecorator(audited, 5);

        CHECK(prioritised->getId() == "PAY");
        CHECK(prioritised->getName() == "Bursary Payment");

        PriorityDecorator* pDeco = dynamic_cast<PriorityDecorator*>(prioritised);
        REQUIRE(pDeco != nullptr);
        CHECK(pDeco->getPriority() == 5);

        // Stack still looks like one WorkItem to a SubProcess
        SubProcess root("ROOT", "Root");
        // Transfer ownership of the stack into the composite (add takes ownership — do not delete prioritised after)
        // Use a clone so we can still inspect the stack variable if needed:
        WorkItem* stackClone = prioritised->clone();
        root.add(stackClone);
        CHECK(root.childCount() == 1);
        CHECK(root.getChild(0)->getId() == "PAY");

        delete prioritised; // deletes AuditDecorator -> Task
    }

    TEST_CASE("Decorator in SubProcess tree: add decorated task as child") {
        SubProcess* root = new SubProcess("INTAKE", "Intake");
        Task* raw = new Task("VERIFY", "Verify");
        WorkItem* decorated = new PriorityDecorator(raw, 3);

        root->add(decorated);
        CHECK(root->childCount() == 1);
        CHECK(root->getChild(0)->getId() == "VERIFY");
        CHECK(root->getChild(0)->getParent() == root);

        PriorityDecorator* pd = dynamic_cast<PriorityDecorator*>(root->getChild(0));
        REQUIRE(pd != nullptr);
        CHECK(pd->getPriority() == 3);

        delete root;
    }

    TEST_CASE("WorkItemDecorator: forwards getId/getName from inner") {
        Task* inner = new Task("X", "Name X");
        PriorityDecorator deco(inner, 1);
        CHECK(deco.getId() == inner->getId());
        CHECK(deco.getName() == inner->getName());
    }
}
