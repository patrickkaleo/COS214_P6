/**
 * @file DataRuleRouting.cpp
 * @brief Implementation of data-driven dynamic task routing strategy.
 * @author Enock Nyamweya
 * @version 2.0
 */

#include "DataRuleRouting.h"
#include "ParticipantDirectory.h"
#include "Participant.h"
#include "InstanceData.h"

DataRuleRouting::DataRuleRouting()
    : field(""), valueToRole(), fallbackRole("") {
}

DataRuleRouting::DataRuleRouting(const std::string& fieldName,
                                 const std::map<std::string, std::string>& mapping,
                                 const std::string& defaultFallbackRole)
    : field(fieldName), valueToRole(mapping), fallbackRole(defaultFallbackRole) {
}

std::string DataRuleRouting::resolveRole(const InstanceData& data) const {
    if (!field.empty() && data.has(field)) {
        const std::map<std::string, std::string>::const_iterator found = valueToRole.find(data.get(field));
        if (found != valueToRole.end()) {
            return found->second;
        }
    }
    return fallbackRole;
}

std::string DataRuleRouting::roleFor(const InstanceData& data) const {
    return resolveRole(data);
}

Participant* DataRuleRouting::select(const Task& task,
                                     const InstanceData& data,
                                     const ParticipantDirectory& directory) {
    (void)task;

    const std::string targetRole = resolveRole(data);

    // If no target role could be resolved, no candidate can be chosen
    if (targetRole.empty()) {
        return nullptr;
    }

    // Query participant directory for candidates matching target role
    std::vector<std::shared_ptr<Participant>> candidates = directory.withRole(targetRole);
    if (candidates.empty()) {
        return nullptr;
    }

    return candidates.front().get();
}

RoutingStrategy* DataRuleRouting::clone() const {
    return new DataRuleRouting(*this);
}

std::string DataRuleRouting::getField() const {
    return field;
}

void DataRuleRouting::setField(const std::string& fieldName) {
    field = fieldName;
}

std::map<std::string, std::string> DataRuleRouting::getMapping() const {
    return valueToRole;
}

void DataRuleRouting::addRule(const std::string& value, const std::string& roleName) {
    valueToRole[value] = roleName;
}

std::string DataRuleRouting::getFallbackRole() const {
    return fallbackRole;
}

void DataRuleRouting::setFallbackRole(const std::string& defaultRole) {
    fallbackRole = defaultRole;
}
