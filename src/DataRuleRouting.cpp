/**
 * @file DataRuleRouting.cpp
 * @brief Implementation of data-driven dynamic task routing strategy.
 * @author Enock Nyamweya
 * @version 1.0
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

Participant* DataRuleRouting::select(const Task& task,
                                     const InstanceData& data,
                                     const ParticipantDirectory& directory) {
    (void)task;

    std::string targetRole = "";

    // If field is configured and present in data, check mapping rules
    if (!field.empty() && data.has(field)) {
        std::string fieldValue = data.get(field);
        auto it = valueToRole.find(fieldValue);
        if (it != valueToRole.end()) {
            targetRole = it->second;
        } else {
            // Unmapped value: use fallback
            targetRole = fallbackRole;
        }
    } else {
        // Field missing or unconfigured: use fallback
        targetRole = fallbackRole;
    }

    // If no target role could be resolved, no candidate can be chosen
    if (targetRole.empty()) {
        return nullptr;
    }

    // Query participant directory for candidates matching target role
    std::vector<Participant*> candidates = directory.withRole(targetRole);
    if (candidates.empty()) {
        return nullptr;
    }

    return candidates.front();
}

RoutingStrategy* DataRuleRouting::clone() const {
    return new DataRuleRouting(*this);
}

void DataRuleRouting::addRule(const std::string& value, const std::string& roleName) {
    valueToRole[value] = roleName;
}

std::string DataRuleRouting::getField() const {
    return field;
}

void DataRuleRouting::setField(const std::string& fieldName) {
    field = fieldName;
}

std::string DataRuleRouting::getFallbackRole() const {
    return fallbackRole;
}

void DataRuleRouting::setFallbackRole(const std::string& defaultFallbackRole) {
    fallbackRole = defaultFallbackRole;
}

const std::map<std::string, std::string>& DataRuleRouting::getRuleMap() const {
    return valueToRole;
}
