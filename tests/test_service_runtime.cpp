#include "doctest.h"
#include "Task.h"
#include "Participant.h"
#include "ParticipantDirectory.h"
#include "ServiceRegistry.h"
#include "ExternalService.h"
#include "GovernmentIDAdapter.h"
#include "BankGatewayAdapter.h"
#include "ApprovalRouting.h"
#include "ConcreteApprovalHandler.h"
#include "ThresholdRule.h"
#include "AlwaysAcceptRule.h"
#include "AuditDecorator.h"
#include "AuditLogger.h"

#include <stdexcept>
#include <memory>

namespace {
class RuntimeRecordingService : public ExternalService {
public:
    int calls = 0;
    ServiceRequest request;
    ServiceResult response;
    bool throws = false;

    std::string name() const override { return "RuntimeRecordingService"; }
    ServiceResult invoke(const ServiceRequest& value) override {
        ++calls;
        request = value;
        if (throws) throw std::runtime_error("simulated unavailable adapter");
        return response;
    }
};

void startRuntimeTask(Task& task, Participant& actor) {
    REQUIRE(task.makeAvailable());
    REQUIRE(task.assign(&actor));
    REQUIRE(task.start());
}
}

TEST_CASE("Service runtime passes data and applies adapter outcome exactly once") {
    ServiceRegistry services;
    RuntimeRecordingService* adapter = new RuntimeRecordingService();
    adapter->response = ServiceResult::ok("checked", {{"verified", "true"}}, "verified");
    services.add(adapter);
    Participant actor("service-worker", "Service worker");
    InstanceData data({{"accountNumber", "62012345678"}, {"amount", "1500.75"}});
    Task task("VERIFY", "Verify");
    task.setServiceRegistry(&services);
    task.setServiceName(adapter->name());

    CHECK_FALSE(task.executeService(data, "forged-outcome"));
    CHECK(adapter->calls == 0);
    startRuntimeTask(task, actor);
    REQUIRE(task.executeService(data, "forged-outcome"));
    CHECK(adapter->request.operation == "RuntimeRecordingService");
    CHECK(adapter->request.get("accountNumber") == "62012345678");
    CHECK(adapter->request.get("amount") == "1500.75");
    CHECK(task.getStateName() == "Completed");
    CHECK(task.getOutcome() == "verified");
    CHECK(data.get("verified") == "true");
    CHECK(data.get("service.VERIFY.message") == "checked");
    CHECK_FALSE(task.serviceFailed());
    CHECK_FALSE(task.executeService(data, "repeat"));
    CHECK(adapter->calls == 1);
}

TEST_CASE("Service runtime failures discard outputs and permit explicit retry") {
    ServiceRegistry services;
    RuntimeRecordingService* adapter = new RuntimeRecordingService();
    adapter->response = ServiceResult::failure("simulated offline");
    adapter->response.outputs["balance"] = "corrupt partial output";
    services.add(adapter);
    Participant actor("service-worker", "Service worker");
    InstanceData data;
    data.set("balance", "original");
    Task task("PAY", "Pay");
    task.setCheckpoint(true);
    task.setServiceRegistry(&services);
    task.setServiceName(adapter->name());
    startRuntimeTask(task, actor);

    REQUIRE(task.executeService(data, "ignored"));
    CHECK(task.getStateName() == "Escalated");
    CHECK(task.getAssignee() == nullptr);
    CHECK(task.serviceFailed());
    CHECK(task.lastServiceError() == "simulated offline");
    CHECK(data.get("balance") == "original");
    CHECK_FALSE(task.executeService(data, "repeat without reassignment"));
    CHECK(adapter->calls == 1);

    REQUIRE(task.assign(&actor));
    REQUIRE(task.start());
    adapter->response = ServiceResult::ok("retried", {{"balance", "updated"}}, "paid");
    REQUIRE(task.executeService(data, "ignored"));
    CHECK(task.getOutcome() == "paid");
    CHECK(data.get("balance") == "updated");
    CHECK_FALSE(task.serviceFailed());
    CHECK(task.lastServiceError().empty());
    CHECK(adapter->calls == 2);
}

TEST_CASE("Service runtime missing and throwing adapters fail safely") {
    Participant actor("service-worker", "Service worker");
    InstanceData data;
    Task missing("MISSING", "Missing adapter");
    missing.setServiceName("unknown");
    startRuntimeTask(missing, actor);
    REQUIRE(missing.executeService(data, "ok"));
    CHECK(missing.getStateName() == "Escalated");
    CHECK(missing.lastServiceError() == "Service is not registered: unknown");

    ServiceRegistry services;
    RuntimeRecordingService* adapter = new RuntimeRecordingService();
    adapter->throws = true;
    services.add(adapter);
    Task throwing("THROWING", "Throwing adapter");
    throwing.setServiceRegistry(&services);
    throwing.setServiceName(adapter->name());
    startRuntimeTask(throwing, actor);
    REQUIRE(throwing.executeService(data, "ok"));
    CHECK(throwing.getStateName() == "Escalated");
    CHECK(throwing.lastServiceError().find("simulated unavailable adapter") != std::string::npos);
}

TEST_CASE("Service runtime rejects negative identity decisions and completes valid identity") {
    ServiceRegistry services;
    services.add(new GovernmentIDAdapter());
    Participant actor("identity-worker", "Identity worker");
    InstanceData data({{"idNumber", "9999999999999"}, {"surname", "Nobody"}});
    Task rejected("BAD_ID", "Verify identity");
    rejected.setServiceRegistry(&services);
    rejected.setServiceName("IdentityVerificationService");
    startRuntimeTask(rejected, actor);
    REQUIRE(rejected.executeService(data, "approved"));
    CHECK(rejected.getStateName() == "Rejected");
    CHECK(data.get("identityVerified") == "false");
    CHECK(data.get("identityStatus") == "NOT_FOUND");
    CHECK_FALSE(rejected.serviceFailed());

    Task verified("GOOD_ID", "Verify identity");
    verified.setServiceRegistry(&services);
    verified.setServiceName("IdentityVerificationService");
    data.set("idNumber", "0501015800087");
    data.set("surname", "Mokoena");
    startRuntimeTask(verified, actor);
    verified.performWork(data);
    CHECK(verified.getStateName() == "Completed");
    CHECK(verified.getOutcome() == "verified");
    CHECK(data.get("identityVerified") == "true");
}

TEST_CASE("Payment adapter rejects amounts outside integer range without converting them") {
    BankGatewayAdapter adapter;
    for (const char* amount : {"1e300", "-1e300", "nan", "inf", "9223372036854775808"}) {
        ServiceResult result = adapter.invoke(ServiceRequest("pay", {
            {"accountNumber", "62012345678"}, {"branchCode", "250655"},
            {"reference", "SIMULATED"}, {"amount", amount}}));
        CHECK_FALSE(result.success);
        CHECK(result.outputs.empty());
    }
}

TEST_CASE("Service lifecycle transitions pass through the outer audit decorator") {
    ServiceRegistry services;
    RuntimeRecordingService* adapter = new RuntimeRecordingService();
    services.add(adapter);
    Participant actor("audited-worker", "Audited worker");
    InstanceData data;
    AuditLogger logger;
    for (int outcome = 0; outcome < 3; ++outcome) {
        Task* task = new Task("AUDITED", "Audited service");
        task->setServiceRegistry(&services);
        task->setServiceName(adapter->name());
        AuditDecorator outer(task, &logger);
        outer.setInstanceId("service-instance");
        REQUIRE(outer.makeAvailable());
        REQUIRE(outer.assign(&actor));
        REQUIRE(outer.start());
        logger.clearLog();
        if (outcome == 0) adapter->response = ServiceResult::ok("done", {}, "done");
        else if (outcome == 1) adapter->response = ServiceResult::ok("declined", {}, "rejected");
        else adapter->response = ServiceResult::failure("offline");
        REQUIRE(task->executeService(data, "ignored", &outer));
        REQUIRE(logger.getEntryCount() == 1);
        CHECK(logger.getAuditTrail().back().find("INSTANCE:service-instance") != std::string::npos);
        if (outcome == 0) CHECK(outer.getStateName() == "Completed");
        else if (outcome == 1) CHECK(outer.getStateName() == "Rejected");
        else CHECK(outer.getStateName() == "Escalated");
    }

    Task target("WRONG", "Wrong transition target");
    Task actual("ACTUAL", "Actual service");
    actual.setServiceRegistry(&services);
    actual.setServiceName(adapter->name());
    startRuntimeTask(actual, actor);
    int calls = adapter->calls;
    CHECK_FALSE(actual.executeService(data, "ignored", &target));
    CHECK(adapter->calls == calls);
}

TEST_CASE("Approval deadlines follow the handler that actually accepts the request") {
    ParticipantDirectory directory;
    directory.addParticipant(new Participant("officer", "Officer", "Officer"));
    directory.addParticipant(new Participant("manager", "Manager", "Manager"));
    directory.addParticipant(new Participant("committee", "Committee", "Committee"));
    ConcreteApprovalHandler* head = new ConcreteApprovalHandler("Officer", new ThresholdRule("amount", 1000), 1);
    head->setNext(new ConcreteApprovalHandler("Manager", new AlwaysAcceptRule(), 2))
        ->setNext(new ConcreteApprovalHandler("Committee", new AlwaysAcceptRule(), 3));
    InstanceData data;
    data.set("amount", "1500");
    Task approve("TIMED", "Timed approval");
    approve.setRouting(new ApprovalRouting(head));
    approve.setRoutingContext(&data, &directory);
    REQUIRE(approve.makeAvailable());
    REQUIRE(approve.getAssignee() != nullptr);
    CHECK(approve.getAssignee()->getId() == "manager");
    CHECK(approve.getDeadlineHours() == 2);
    CHECK(approve.getEscalationLevel() == 1);
    REQUIRE(approve.escalate());
    REQUIRE(approve.route());
    CHECK(approve.getAssignee()->getId() == "committee");
    CHECK(approve.getDeadlineHours() == 3);

    Task explicitDeadline("EXPLICIT", "Configured deadline");
    CHECK(explicitDeadline.getDeadlineHours() == 0);
    explicitDeadline.setDeadlineHours(4);
    CHECK(explicitDeadline.getDeadlineHours() == 4);
    std::unique_ptr<WorkItem> cloned(explicitDeadline.clone());
    CHECK(static_cast<Task*>(cloned.get())->getDeadlineHours() == 4);
}
