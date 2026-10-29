/**
 * @file Task.cpp
 * @brief Leaf of the Composite pattern — atomic work item.
 * @version 1.0
 */

#include "Task.h"
#include "WorkItem.h"
#include "RoutingStrategy.h"
#include "InstanceData.h"
#include "ParticipantDirectory.h"
#include "Participant.h"
#include "ExternalService.h"
#include "ServiceRegistry.h"
// #include "ServiceRequest.h"
// #include "ServiceResult.h"

#include <string>

using namespace std;

Task::Task(const string& id, const string& itemName)
    : WorkItem(id, itemName),
      routing_(nullptr),
      serviceName_(""),
      checkpoint_(false),
      instanceData_(nullptr),
      directory_(nullptr),
      services_(nullptr)
{}

Task::~Task() {
    delete routing_;
    // instanceData_, directory_, services_ are not owned
}

// Composite hook — called from WorkItem::makeAvailable() after state accepts
void Task::onAvailable() {
    if (!routing_) return;
    if (!instanceData_ || !directory_) return;

    Participant* p = routing_->select(*this, *instanceData_, *directory_);
    if (p) {
        if (assign(p)) {
            setAssignee(p);
        }
    } else {
        // No eligible participant hence escalate (State handles transition and event)
        escalate();
    }
}

// Prototype
WorkItem* Task::clone() const {
    Task* copy = new Task(getId(), getName());
    copy->setOutcome(getOutcome());
    copy->setInstanceId(getInstanceId());
    copy->serviceName_ = serviceName_;
    copy->checkpoint_ = checkpoint_;

    if (routing_) {
        copy->routing_ = routing_->clone();
    }
    // instanceData_, directory_, services_, mediator, parent set by instance after clone
    return copy;
}

// Task-specific configuration
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

    if (!services_) {
        // Cannot resolve adapter (escalate for manual handling)
        escalate();
        return;
    }

    ExternalService* svc = services_->find(serviceName_);
    if (!svc) {
        escalate();
        return;
    }

    // Build request from InstanceData key/value map
    ServiceRequest req;
    // If ServiceRequest has operation + params map:
    // req.operation = serviceName_;
    // req.params = data.getMap();

    ServiceResult result = svc->invoke(req);

    if (result.success) {
        // adapter outputs back into instance data:
        // for (const auto& kv : result.values) {
        //     data.set(kv.first, kv.second);
        // }
        complete("ok");
    } else {
        // Task only signals failure via escalation
        escalate();
    }
}
