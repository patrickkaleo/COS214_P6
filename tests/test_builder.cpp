/**
 * @file test_builder.cpp
 * @brief Unit tests for the Builder and Director workflow construction flow.
 */

#include "doctest.h"

#include "WorkflowDirector.h"
#include "FileWorkflowDefinitionBuilder.h"
#include "AuditDecorator.h"
#include "PriorityDecorator.h"
#include "WorkItemDecorator.h"
#include "RoleBasedRouting.h"
#include "DataRuleRouting.h"

namespace {

const char* const kBuilderFile = "tests/builder.txt";

WorkflowDefinition* buildFromFile(const std::string& filePath) {
	FileWorkflowDefinitionBuilder builder(filePath);
	WorkflowDirector director;
	return director.construct(&builder);
}

Task* taskUnder(WorkItem* node) {
	while (WorkItemDecorator* layer = dynamic_cast<WorkItemDecorator*>(node)) {
		node = layer->getInner();
	}
	return dynamic_cast<Task*>(node);
}

}

TEST_SUITE("Patrick Simuyemba - Builder design pattern ") {

	TEST_CASE("Builder: created definition matches the workflow structure from builder.txt") {
		FileWorkflowDefinitionBuilder builder(kBuilderFile);
		WorkflowDirector director;

		WorkflowDefinition* definition = director.construct(&builder);
		REQUIRE(definition != nullptr);
		CHECK(builder.getResult() == nullptr);
		CHECK(definition->getId() == "WF-100");
		CHECK(definition->getName() == "Onboarding");
		CHECK(definition->getVersion() == 1);

		REQUIRE(definition->getRoot() != nullptr);
		SubProcess* intake = dynamic_cast<SubProcess*>(definition->getRoot());
		REQUIRE(intake != nullptr);
		CHECK(intake->getId() == "INTAKE");
		CHECK(intake->getName() == "Application Intake");
		CHECK(intake->childCount() == 4);

		Task* capture = dynamic_cast<Task*>(intake->getChild(0));
		REQUIRE(capture != nullptr);
		CHECK(capture->getId() == "CAPTURE");
		CHECK(capture->getName() == "Capture Application");
		CHECK(capture->getServiceName() == "Document Verification Service");
		CHECK(capture->isCheckpoint() == true);
		CHECK(capture->getRouting() != nullptr);

		WorkItem* verifyNode = intake->getChild(1);
		REQUIRE(verifyNode != nullptr);
		CHECK(verifyNode->getId() == "VERIFY");
		CHECK(verifyNode->getName() == "Verify Application");

		AuditDecorator* audited = dynamic_cast<AuditDecorator*>(verifyNode);
		REQUIRE(audited != nullptr);
		CHECK(audited->getParent() == intake);
		PriorityDecorator* prioritised = dynamic_cast<PriorityDecorator*>(audited->getInner());
		REQUIRE(prioritised != nullptr);
		CHECK(prioritised->getPriority() == 2);

		Task* verify = dynamic_cast<Task*>(prioritised->getInner());
		REQUIRE(verify != nullptr);
		CHECK(verify->getId() == "VERIFY");
		CHECK(verify->getName() == "Verify Application");
		CHECK(verify->getRouting() != nullptr);
		CHECK(verify->getEscalationLevel() == 2);

		SubProcess* review = dynamic_cast<SubProcess*>(intake->getChild(2));
		REQUIRE(review != nullptr);
		CHECK(review->getId() == "REVIEW");
		CHECK(review->getName() == "Faculty Review");
		CHECK(review->getParent() == intake);
		CHECK(review->childCount() == 2);

		Task* notify = dynamic_cast<Task*>(intake->getChild(3));
		REQUIRE(notify != nullptr);
		CHECK(notify->getId() == "NOTIFY");
		CHECK(notify->getName() == "Notify Applicant");

		CHECK(definition->getRules().size() == 5);
		REQUIRE(!definition->getRules().empty());
		CHECK(definition->getRules()[0]->fromId == "CAPTURE");
		CHECK(definition->getRules()[0]->toId == "VERIFY");
		CHECK(definition->getRules()[0]->outcome == "approved");

		delete definition;
	}

	TEST_CASE("Builder: nested review process holds audit-only and priority-only tasks") {
		WorkflowDefinition* definition = buildFromFile(kBuilderFile);
		REQUIRE(definition != nullptr);
		SubProcess* intake = dynamic_cast<SubProcess*>(definition->getRoot());
		REQUIRE(intake != nullptr);
		SubProcess* review = dynamic_cast<SubProcess*>(intake->getChild(2));
		REQUIRE(review != nullptr);

		AuditDecorator* screenAudit = dynamic_cast<AuditDecorator*>(review->getChild(0));
		REQUIRE(screenAudit != nullptr);
		CHECK(screenAudit->getId() == "SCREEN");
		CHECK(screenAudit->getParent() == review);
		CHECK(dynamic_cast<PriorityDecorator*>(screenAudit->getInner()) == nullptr);
		Task* screen = dynamic_cast<Task*>(screenAudit->getInner());
		REQUIRE(screen != nullptr);
		CHECK(screen->getName() == "Screen Documents");
		CHECK(screen->isCheckpoint() == false);
		CHECK(screen->getEscalationLevel() == 0);
		CHECK(screen->getServiceName().empty());

		PriorityDecorator* decidePriority = dynamic_cast<PriorityDecorator*>(review->getChild(1));
		REQUIRE(decidePriority != nullptr);
		CHECK(decidePriority->getId() == "DECIDE");
		CHECK(decidePriority->getPriority() == 5);
		CHECK(decidePriority->getParent() == review);
		CHECK(dynamic_cast<AuditDecorator*>(decidePriority->getInner()) == nullptr);
		Task* decide = dynamic_cast<Task*>(decidePriority->getInner());
		REQUIRE(decide != nullptr);
		CHECK(decide->getName() == "Decide Outcome");
		CHECK(decide->isCheckpoint() == true);
		CHECK(decide->getEscalationLevel() == 5);
		CHECK(decide->getServiceName().empty());

		delete definition;
	}

	TEST_CASE("Builder: role and data routing are attached to the named tasks") {
		WorkflowDefinition* definition = buildFromFile(kBuilderFile);
		REQUIRE(definition != nullptr);
		SubProcess* intake = dynamic_cast<SubProcess*>(definition->getRoot());
		REQUIRE(intake != nullptr);

		Task* capture = dynamic_cast<Task*>(intake->getChild(0));
		REQUIRE(capture != nullptr);
		RoleBasedRouting* captureRoute = dynamic_cast<RoleBasedRouting*>(capture->getRouting());
		REQUIRE(captureRoute != nullptr);
		CHECK(captureRoute->getRole() == "Admissions Officer");

		Task* verify = taskUnder(intake->getChild(1));
		REQUIRE(verify != nullptr);
		DataRuleRouting* verifyRoute = dynamic_cast<DataRuleRouting*>(verify->getRouting());
		REQUIRE(verifyRoute != nullptr);
		CHECK(verifyRoute->getField() == "severity");
		CHECK(verifyRoute->getFallbackRole() == "Exception Analyst");
		CHECK(verifyRoute->getMapping().at("low") == "Warehouse Clerk");
		CHECK(verifyRoute->getMapping().at("high") == "Exception Analyst");

		SubProcess* review = dynamic_cast<SubProcess*>(intake->getChild(2));
		REQUIRE(review != nullptr);
		Task* screen = taskUnder(review->getChild(0));
		REQUIRE(screen != nullptr);
		RoleBasedRouting* screenRoute = dynamic_cast<RoleBasedRouting*>(screen->getRouting());
		REQUIRE(screenRoute != nullptr);
		CHECK(screenRoute->getRole() == "Faculty Clerk");

		Task* decide = taskUnder(review->getChild(1));
		REQUIRE(decide != nullptr);
		DataRuleRouting* decideRoute = dynamic_cast<DataRuleRouting*>(decide->getRouting());
		REQUIRE(decideRoute != nullptr);
		CHECK(decideRoute->getField() == "risk");
		CHECK(decideRoute->getFallbackRole() == "Risk Officer");
		CHECK(decideRoute->getMapping().at("low") == "Reviewer");
		CHECK(decideRoute->getMapping().at("high") == "Senior Reviewer");

		Task* notify = dynamic_cast<Task*>(intake->getChild(3));
		REQUIRE(notify != nullptr);
		CHECK(notify->getRouting() == nullptr);
		CHECK(notify->getServiceName() == "Email Notification Service");
		CHECK(notify->isCheckpoint() == false);
		CHECK(notify->getEscalationLevel() == 0);

		delete definition;
	}

	TEST_CASE("Builder: dependency rules keep file order and skipped lines add nothing") {
		WorkflowDefinition* definition = buildFromFile(kBuilderFile);
		REQUIRE(definition != nullptr);
		SubProcess* intake = dynamic_cast<SubProcess*>(definition->getRoot());
		REQUIRE(intake != nullptr);

		const std::vector<DependencyRule*> rules = definition->getRules();
		REQUIRE(rules.size() == 5);
		CHECK(rules[1]->fromId == "VERIFY");
		CHECK(rules[1]->toId == "SCREEN");
		CHECK(rules[1]->outcome == "approved");
		CHECK(rules[2]->fromId == "SCREEN");
		CHECK(rules[2]->toId == "DECIDE");
		CHECK(rules[2]->outcome == "complete");
		CHECK(rules[3]->fromId == "DECIDE");
		CHECK(rules[3]->toId == "NOTIFY");
		CHECK(rules[3]->outcome == "approved");
		CHECK(rules[4]->fromId == "DECIDE");
		CHECK(rules[4]->toId == "NOTIFY");
		CHECK(rules[4]->outcome == "rejected");

		CHECK(intake->find("BROKEN") == nullptr);
		CHECK(intake->find("GHOST") == nullptr);
		CHECK(intake->find("MISSING") == nullptr);
		CHECK(intake->find("SCREEN") != nullptr);
		CHECK(intake->find("DECIDE") != nullptr);

		delete definition;
	}

	TEST_CASE("Builder: instantiateTree copies the decorated nested structure") {
		WorkflowDefinition* definition = buildFromFile(kBuilderFile);
		REQUIRE(definition != nullptr);
		SubProcess* intake = dynamic_cast<SubProcess*>(definition->getRoot());
		REQUIRE(intake != nullptr);

		WorkItem* copy = definition->instantiateTree();
		SubProcess* copiedIntake = dynamic_cast<SubProcess*>(copy);
		REQUIRE(copiedIntake != nullptr);
		CHECK(copiedIntake != intake);
		CHECK(copiedIntake->getId() == "INTAKE");
		CHECK(copiedIntake->childCount() == 4);

		AuditDecorator* copiedVerify = dynamic_cast<AuditDecorator*>(copiedIntake->getChild(1));
		REQUIRE(copiedVerify != nullptr);
		CHECK(copiedVerify != intake->getChild(1));
		PriorityDecorator* copiedPriority = dynamic_cast<PriorityDecorator*>(copiedVerify->getInner());
		REQUIRE(copiedPriority != nullptr);
		CHECK(copiedPriority->getPriority() == 2);
		Task* copiedTask = dynamic_cast<Task*>(copiedPriority->getInner());
		REQUIRE(copiedTask != nullptr);
		CHECK(copiedTask->getEscalationLevel() == 2);
		CHECK(copiedTask->getRouting() != nullptr);
		CHECK(copiedTask->getRouting() != taskUnder(intake->getChild(1))->getRouting());

		SubProcess* copiedReview = dynamic_cast<SubProcess*>(copiedIntake->getChild(2));
		REQUIRE(copiedReview != nullptr);
		CHECK(copiedReview->childCount() == 2);
		CHECK(copiedReview->getParent() == copiedIntake);
		CHECK(dynamic_cast<AuditDecorator*>(copiedReview->getChild(0)) != nullptr);
		PriorityDecorator* copiedDecide = dynamic_cast<PriorityDecorator*>(copiedReview->getChild(1));
		REQUIRE(copiedDecide != nullptr);
		CHECK(copiedDecide->getPriority() == 5);
		CHECK(taskUnder(copiedDecide)->isCheckpoint() == true);

		delete copy;
		delete definition;
	}

	TEST_CASE("Builder: a missing file yields a definition with no root") {
		WorkflowDefinition* definition = buildFromFile("tests/no-such-builder.txt");
		REQUIRE(definition != nullptr);
		CHECK(definition->getId().empty());
		CHECK(definition->getName().empty());
		CHECK(definition->getRoot() == nullptr);
		CHECK(definition->getRules().empty());
		delete definition;
	}

	TEST_CASE("Builder: a second live definition cannot reuse the same id") {
		WorkflowDefinition* first = buildFromFile(kBuilderFile);
		REQUIRE(first != nullptr);
		CHECK(first->getId() == "WF-100");

		WorkflowDefinition* second = buildFromFile(kBuilderFile);
		REQUIRE(second != nullptr);
		CHECK(second->getId().empty());
		CHECK(second->getName() == "Onboarding");
		CHECK(second->getRoot() != nullptr);
		delete second;
		delete first;

		WorkflowDefinition* third = buildFromFile(kBuilderFile);
		REQUIRE(third != nullptr);
		CHECK(third->getId() == "WF-100");
		delete third;
	}
}
