/**
 * @file test_builder.cpp
 * @brief Unit tests for the Builder and Director workflow construction flow.
 */

#include "doctest.h"

#include "WorkflowDirector.h"
#include "FileWorkflowDefinitionBuilder.h"
#include "AuditDecorator.h"
#include "PriorityDecorator.h"

TEST_SUITE("Patrick Simuyemba - Builder design pattern ") {

	TEST_CASE("Builder: created definition matches the workflow structure from builder.txt") {
		const std::string filePath = "tests/builder.txt";
		FileWorkflowDefinitionBuilder builder(filePath);
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
		CHECK(intake->childCount() == 2);

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
		PriorityDecorator* prioritised = dynamic_cast<PriorityDecorator*>(audited->getInner());
		REQUIRE(prioritised != nullptr);
		CHECK(prioritised->getPriority() == 2);

		Task* verify = dynamic_cast<Task*>(prioritised->getInner());
		REQUIRE(verify != nullptr);
		CHECK(verify->getId() == "VERIFY");
		CHECK(verify->getName() == "Verify Application");
		CHECK(verify->getRouting() != nullptr);
		CHECK(verify->getEscalationLevel() == 2);

		CHECK(definition->getRules().size() == 1);
		REQUIRE(!definition->getRules().empty());
		CHECK(definition->getRules()[0]->fromId == "CAPTURE");
		CHECK(definition->getRules()[0]->toId == "VERIFY");
		CHECK(definition->getRules()[0]->outcome == "approved");

		delete definition;
	}
}

