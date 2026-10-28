/**
 * @file test_observer_strategy.cpp
 * @brief Unit tests for Observer & Strategy subsystems and Participant Directory.
 * @author Enock Nyamweya
 */

#include "doctest.h"

#include <memory>
#include <vector>
#include <string>
#include <map>

#include "EventDispatcher.h"
#include "NotificationService.h"
#include "AuditLogger.h"
#include "MonitoringService.h"

#include "RoutingStrategy.h"
#include "RoleBasedRouting.h"
#include "DataRuleRouting.h"
#include "ApprovalRouting.h"
#include "ApprovalHandler.h"
#include "ApprovalRequest.h"
#include "Participant.h"
#include "ParticipantDirectory.h"
#include "InstanceData.h"

TEST_SUITE("Patrick Simuyemba - Builder design pattern ") {

	TEST_CASE("start defintion") {
		
	}

	TEST_CASE("Observer: verifies observer registration and push event dispatching") {
		EventDispatcher& dispatcher = EventDispatcher::getInstance();
		dispatcher.clearListeners();

		auto notifier = std::make_shared<NotificationService>("TestChannel", false);
		auto auditor = std::make_shared<AuditLogger>(false);
		auto monitor = std::make_shared<MonitoringService>();

		dispatcher.attachListener(notifier);
		dispatcher.attachListener(auditor);
		dispatcher.attachListener(monitor);

		CHECK(dispatcher.getListenerCount() == 3);

		dispatcher.notifyListeners(WorkflowEvent(
			EventType::AVAILABLE, "WF-101", "TASK-1", "Registration"
		));
		dispatcher.notifyListeners(WorkflowEvent(
			EventType::STARTED, "WF-101", "TASK-1", "Registration"
		));
		dispatcher.notifyListeners(WorkflowEvent(
			EventType::COMPLETED, "WF-101", "TASK-1", "Registration", "Verified"
		));

		CHECK(notifier->getNotificationCount() == 3);
		CHECK(auditor->getEntryCount() == 3);
		CHECK(monitor->getTotalEvents() == 3);
		CHECK(monitor->getCountForType(EventType::COMPLETED) == 1);

		dispatcher.clearListeners();
	}

	TEST_CASE("Observer: verifies dynamic listener detaching with raw pointer compatibility") {
		EventDispatcher& dispatcher = EventDispatcher::getInstance();
		dispatcher.clearListeners();

		NotificationService notifier("Channel", false);
		AuditLogger auditor(false);

		dispatcher.attachListener(&notifier);
		dispatcher.attachListener(&auditor);
		CHECK(dispatcher.getListenerCount() == 2);

		dispatcher.detachListener(&notifier);
		CHECK(dispatcher.getListenerCount() == 1);

		dispatcher.notifyListeners(WorkflowEvent(
			EventType::STARTED, "WF-102", "TASK-2", "Review"
		));

		CHECK(notifier.getNotificationCount() == 0);
		CHECK(auditor.getEntryCount() == 1);

		dispatcher.clearListeners();
	}

	TEST_CASE("AuditLogger: verifies immutable audit log formatting") {
		AuditLogger auditor(false);
		WorkflowEvent ev(EventType::COMPLETED, "WF-103", "TASK-3", "Verification", "Verified");

		auditor.onEvent(ev);
		CHECK(auditor.getEntryCount() == 1);

		const auto& entries = auditor.getAuditTrail();
		CHECK(entries.size() == 1);
		CHECK(entries[0].find("COMPLETED") != std::string::npos);
		CHECK(entries[0].find("WF-103") != std::string::npos);
		CHECK(entries[0].find("TASK-3") != std::string::npos);
	}

	TEST_CASE("MonitoringService: verifies real-time failure rate calculation") {
		MonitoringService monitor;

		monitor.onEvent(WorkflowEvent(EventType::STARTED, "WF-001", "TASK-1", "Step 1"));
		CHECK(monitor.getActiveInstanceCount() == 1);

		monitor.onEvent(WorkflowEvent(EventType::INSTANCE_COMPLETED, "WF-001", "", "WorkflowInstance", "Success"));
		CHECK(monitor.getActiveInstanceCount() == 0);
		CHECK(monitor.getCompletedInstanceCount() == 1);

		monitor.onEvent(WorkflowEvent(EventType::STARTED, "WF-002", "TASK-1", "Step 1"));
		monitor.onEvent(WorkflowEvent(EventType::FAILED, "WF-002", "TASK-1", "Step 1", "Error"));
		CHECK(monitor.getTotalEvents() == 4);
		CHECK(monitor.getFailureRate() == doctest::Approx(25.0));
	}
}

class MockApprovalHandler : public ApprovalHandler {
private:
	std::shared_ptr<Participant> approver;
public:
	explicit MockApprovalHandler(std::shared_ptr<Participant> p)
		: approver(p) {}
	explicit MockApprovalHandler(Participant* p)
		: approver(p) {}

	virtual std::shared_ptr<Participant> handle(ApprovalRequest& req) override {
		(void)req;
		return approver;
	}
};

TEST_SUITE("Enock Nyamweya - Strategy Subsystem") {

	TEST_CASE("RoleBasedRouting: selects first available participant holding required role") {
		ParticipantDirectory directory;
		auto alice = std::make_shared<Participant>("p1", "Alice Smith", "Triage Nurse");
		auto bob = std::make_shared<Participant>("p2", "Bob Jones", "Emergency Doctor");
		directory.addParticipant(alice);
		directory.addParticipant(bob);

		RoleBasedRouting strategy("Triage Nurse");
		InstanceData data;

		std::shared_ptr<Participant> selected = strategy.select(getDummyTask(), data, directory);
		REQUIRE(selected != nullptr);
		CHECK(selected->getId() == "p1");
		CHECK(selected->getName() == "Alice Smith");
		CHECK(selected->hasRole("Triage Nurse") == true);
	}

	TEST_CASE("RoleBasedRouting: returns nullptr when no participant holds target role") {
		ParticipantDirectory directory;
		directory.addParticipant(new Participant("p1", "Alice Smith", "Triage Nurse"));

		RoleBasedRouting strategy("Chief Surgeon");
		InstanceData data;

		std::shared_ptr<Participant> selected = strategy.select(getDummyTask(), data, directory);
		CHECK(selected == nullptr);
	}

	TEST_CASE("DataRuleRouting: dynamically maps form data to corresponding roles") {
		ParticipantDirectory directory;
		directory.addParticipant(std::make_shared<Participant>("doc1", "Dr. House", "Emergency Doctor"));
		directory.addParticipant(std::make_shared<Participant>("doc2", "Dr. Watson", "General Practitioner"));
		directory.addParticipant(std::make_shared<Participant>("intern1", "Intern John", "Clinic Intern"));

		std::map<std::string, std::string> mapping = {
			{"critical", "Emergency Doctor"},
			{"mild", "General Practitioner"}
		};
		DataRuleRouting strategy("severity", mapping, "Clinic Intern");

		InstanceData criticalData;
		criticalData.set("severity", "critical");
		std::shared_ptr<Participant> pCritical = strategy.select(getDummyTask(), criticalData, directory);
		REQUIRE(pCritical != nullptr);
		CHECK(pCritical->getId() == "doc1");
		CHECK(pCritical->getName() == "Dr. House");

		InstanceData mildData;
		mildData.set("severity", "mild");
		std::shared_ptr<Participant> pMild = strategy.select(getDummyTask(), mildData, directory);
		REQUIRE(pMild != nullptr);
		CHECK(pMild->getId() == "doc2");
		CHECK(pMild->getName() == "Dr. Watson");
	}

	TEST_CASE("DataRuleRouting: falls back to default role when field is unmapped or missing") {
		ParticipantDirectory directory;
		directory.addParticipant(std::make_shared<Participant>("intern1", "Intern John", "Clinic Intern"));

		std::map<std::string, std::string> mapping = {
			{"critical", "Emergency Doctor"}
		};
		DataRuleRouting strategy("severity", mapping, "Clinic Intern");

		InstanceData unknownData;
		unknownData.set("severity", "unknown_value");
		std::shared_ptr<Participant> pUnknown = strategy.select(getDummyTask(), unknownData, directory);
		REQUIRE(pUnknown != nullptr);
		CHECK(pUnknown->getId() == "intern1");

		InstanceData emptyData;
		std::shared_ptr<Participant> pEmpty = strategy.select(getDummyTask(), emptyData, directory);
		REQUIRE(pEmpty != nullptr);
		CHECK(pEmpty->getId() == "intern1");
	}

	TEST_CASE("ApprovalRouting: delegates to ApprovalHandler Chain of Responsibility") {
		auto dean = std::make_shared<Participant>("p_dean", "Prof. X", "Dean");
		std::unique_ptr<MockApprovalHandler> handler(new MockApprovalHandler(dean));

		ApprovalRouting approvalStrategy(std::move(handler));
		InstanceData data;
		ParticipantDirectory directory;

		std::shared_ptr<Participant> approver = approvalStrategy.select(getDummyTask(), data, directory);
		REQUIRE(approver != nullptr);
		CHECK(approver->getId() == "p_dean");
		CHECK(approver->getName() == "Prof. X");
	}

	TEST_CASE("RoutingStrategy: clone creates independent strategy duplicate") {
		std::map<std::string, std::string> mapping = {
			{"high", "Senior Officer"}
		};
		DataRuleRouting original("priority", mapping, "Junior Officer");

		std::unique_ptr<RoutingStrategy> cloned = original.clone();
		REQUIRE(cloned != nullptr);

		DataRuleRouting* clonedDataRule = dynamic_cast<DataRuleRouting*>(cloned.get());
		REQUIRE(clonedDataRule != nullptr);
		CHECK(clonedDataRule->getField() == "priority");
		CHECK(clonedDataRule->getFallbackRole() == "Junior Officer");

		clonedDataRule->setFallbackRole("Supervisor");
		CHECK(original.getFallbackRole() == "Junior Officer");
		CHECK(clonedDataRule->getFallbackRole() == "Supervisor");
	}
}
