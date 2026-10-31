/**
 * @file Task.cpp
 * @brief Implementation of the Task leaf of the Composite pattern.
 * @version 1.0
 */

#include "Task.h"
#include "RoutingStrategy.h"
#include "Participant.h"
#include "ParticipantDirectory.h"
#include "InstanceData.h"
#include "ServiceRegistry.h"
#include "ExternalService.h"
#include "ServiceRequest.h"
#include "ServiceResult.h"
#include "ApprovalRouting.h"
#include "WorkItemDecorator.h"

#include <iostream>
#include <exception>
#include <memory>

using namespace std;

Task::Task(const string& id, const string& itemName)
    : WorkItem(id, itemName),
      routing_(nullptr),
      serviceName_(""),
      checkpoint_(false),
      deadlineHours_(0),
      instanceData_(nullptr),
      directory_(nullptr),
      services_(nullptr)
{}

Task::~Task() {
    delete routing_;
}

bool Task::route() {
    if (!routing_ || !instanceData_ || !directory_) {
        return false;
    }

    Participant* p = routing_->select(*this, *instanceData_, *directory_);
    if (p) {
        if (assign(p)) {
            const ApprovalRouting* approval = dynamic_cast<const ApprovalRouting*>(routing_);
            if (approval && approval->getSelectedLevel() >= 0) {
                setEscalationLevel(approval->getSelectedLevel());
            }
            setAssignee(p);
            return true;
        }
        return false;
    } else {
        return escalate();
    }
}

void Task::onAvailable() {
    route();
}

WorkItem* Task::clone() const {
    Task* copy = new Task(getId(), getName());
    copy->setOutcome(getOutcome());
    copy->setInstanceId(getInstanceId());
    copy->serviceName_ = serviceName_;
    copy->checkpoint_ = checkpoint_;
    copy->deadlineHours_ = deadlineHours_;
    copy->setEscalationLevel(getEscalationLevel());

    if (routing_) {
        copy->routing_ = routing_->clone();
    }
    return copy;
}

void Task::setRouting(RoutingStrategy* strategy) {
    delete routing_;
    routing_ = strategy;
}

RoutingStrategy* Task::getRouting() const {
    return routing_;
}

void Task::setServiceName(const string& name) {
    serviceName_ = name;
}

string Task::getServiceName() const {
    return serviceName_;
}

void Task::setCheckpoint(bool value) {
    checkpoint_ = value;
}

bool Task::isCheckpoint() const {
    return checkpoint_;
}

void Task::setRoutingContext(const InstanceData* data, const ParticipantDirectory* directory) {
    instanceData_ = data;
    directory_ = directory;
}

void Task::setRoutingContext(InstanceData* data, ParticipantDirectory* directory) {
    instanceData_ = data;
    directory_ = directory;
}

void Task::setServiceRegistry(ServiceRegistry* registry) {
    services_ = registry;
}

void Task::performWork(InstanceData& data) {
    if (serviceName_.empty()) return;
    executeService(data, "ok");
}

bool Task::executeService(InstanceData& data, const string& requestedOutcome,
                          WorkItem* transitionTarget) {
    // Validate before invoking an adapter: refused/repeated commands must never
    // repeat a service operation.
    WorkItem* target = transitionTarget ? transitionTarget : this;
    WorkItem* unwrapped = target;
    while (WorkItemDecorator* decorator = dynamic_cast<WorkItemDecorator*>(unwrapped))
        unwrapped = decorator->getInner();
    if (unwrapped != this || target->getStateName() != "InProgress") return false;
    instanceData_ = &data;
    if (serviceName_.empty()) return target->complete(requestedOutcome);

    ServiceResult result;
    ExternalService* service = services_ ? services_->find(serviceName_) : nullptr;
    if (!service) {
        result = ServiceResult::failure("Service is not registered: " + serviceName_);
    } else {
        try {
            result = service->invoke(ServiceRequest(serviceName_, data.getMap()));
        } catch (const std::exception& error) {
            result = ServiceResult::failure(string("Service failed: ") + error.what());
        } catch (...) {
            result = ServiceResult::failure("Service failed with an unknown error");
        }
    }

    const string key = "service." + getId() + ".";
    if (result.success) result.applyTo(data);
    data.set(key + "success", result.success ? "true" : "false");
    data.set(key + "message", result.message);
    data.set(key + "outcome", result.outcome);
    if (!result.success) return target->escalate();
    if (result.outcome == "rejected") return target->reject(result.message);
    return target->complete(result.outcome.empty() ? requestedOutcome : result.outcome);
}

bool Task::serviceFailed() const {
    return instanceData_ && instanceData_->get("service." + getId() + ".success") == "false";
}

string Task::lastServiceError() const {
    return serviceFailed() ? instanceData_->get("service." + getId() + ".message") : "";
}

void Task::setDeadlineHours(int hours) {
    deadlineHours_ = hours > 0 ? hours : 0;
}

int Task::getDeadlineHours() const {
    if (deadlineHours_ > 0) return deadlineHours_;
    const ApprovalRouting* approval = dynamic_cast<const ApprovalRouting*>(routing_);
    if (!approval) return 0;
    // Re-evaluate against restored data/level instead of retaining a selection
    // cached before undo. The copy keeps inspection free of routing mutations.
    if (instanceData_ && directory_) {
        std::unique_ptr<RoutingStrategy> copy(approval->clone());
        copy->select(*this, *instanceData_, *directory_);
        return static_cast<ApprovalRouting*>(copy.get())->getDeadlineHours();
    }
    return approval->getDeadlineHours();
}
