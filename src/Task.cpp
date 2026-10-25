/**
 * @file Task.cpp
 * @brief Task routing, assignment and copying.
 * @author Tebogo Mokgwatsane, Angelo Anthony
 * @version 1.1
 */

#include "Task.h"
#include "RoutingStrategy.h"

Task::Task(const std::string& id, const std::string& name)
    : WorkItem(id, name),
      routing(),
      serviceName(""),
      checkpoint(false),
      data(nullptr),
      directory(nullptr) {
}

Task::Task(const Task& other)
    : WorkItem(other),
      routing(other.routing ? other.routing->clone() : nullptr),
      serviceName(other.serviceName),
      checkpoint(other.checkpoint),
      data(nullptr),       // bound again by the new instance
      directory(nullptr) {
}

Task::~Task() = default;

Task* Task::clone() const {
    return new Task(*this);
}

bool Task::route() {
    if (!routing || data == nullptr || directory == nullptr) {
        return false;
    }
    Participant* chosen = routing->select(*this, *data, *directory);
    if (chosen != nullptr) {
        return assign(chosen);
    }
    // Nobody can take the work at this level, so hand it to a higher authority.
    return escalate();
}

void Task::onAvailable() {
    route();
}

void Task::setRouting(RoutingStrategy* strategy) {
    routing.reset(strategy);
}

RoutingStrategy* Task::getRouting() const {
    return routing.get();
}

void Task::setServiceName(const std::string& name) {
    serviceName = name;
}

std::string Task::getServiceName() const {
    return serviceName;
}

void Task::setCheckpoint(bool isCheckpoint) {
    checkpoint = isCheckpoint;
}

bool Task::isCheckpoint() const {
    return checkpoint;
}

void Task::setRoutingContext(const InstanceData* instanceData,
                             const ParticipantDirectory* participants) {
    data = instanceData;
    directory = participants;
}
