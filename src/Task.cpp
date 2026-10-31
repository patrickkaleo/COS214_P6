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

#include <iostream>

using namespace std;

Task::Task(const string& id, const string& itemName)
    : WorkItem(id, itemName),
      routing_(nullptr),
      serviceName_(""),
      deadlineHours_(0),
      dueAt_(0),
      checkpoint_(false),
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
            setAssignee(p);
            return true;
        }
        return false;
    } else {
        return escalate();
    }
}

void Task::onAvailable() {
    if (deadlineHours_ > 0) {
        dueAt_ = time(nullptr) + deadlineHours_ * 3600;
    }
    route();
}

void Task::setDeadlineHours(int hours) {
    deadlineHours_ = hours;
}

int Task::getDeadlineHours() const {
    return deadlineHours_;
}

bool Task::checkDeadline(time_t now) {
    if (dueAt_ == 0 || now < dueAt_ || isFinished() || getStateName() == "Escalated") {
        return false;
    }

    Participant* previous = getAssignee();
    setOutcome("Deadline passed");
    if (!escalate()) {
        return false;
    }
    dueAt_ = time(nullptr) + deadlineHours_ * 3600;

    if (routing_ && instanceData_ && directory_) {
        Participant* next = routing_->select(*this, *instanceData_, *directory_);
        if (next && next != previous) {
            assign(next);
        }
    }
    return true;
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
    // Keep a pointer so onAvailable or later steps can still see the same payload
    instanceData_ = &data;

    // No external service configured (nothing to invoke)
    if (serviceName_.empty()) {
        return;
    }

    ExternalService* svc = services_ ? services_->find(serviceName_) : nullptr;
    if (!svc) {
        // Cannot resolve adapter (escalate for manual handling)
        serviceMessage_ = "Service '" + serviceName_ + "' is not registered";
        setOutcome(serviceMessage_);
        escalate();
        return;
    }

    ServiceRequest req(serviceName_, data.getMap());
    ServiceResult result = svc->invoke(req);

    result.applyTo(data);
    serviceMessage_ = result.message;

    if (!result.success) {
        setOutcome(result.message);
        escalate();
    }
}

string Task::getServiceMessage() const {
    return serviceMessage_;
}
