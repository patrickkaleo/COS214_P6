/**
 * @file RoleBasedRouting.cpp
 * @brief Implementation of role-based task allocation strategy.
 * @author Enock Nyamweya
 * @version 1.0
 */

#include "RoleBasedRouting.h"
#include "ParticipantDirectory.h"
#include "Participant.h"
#include "InstanceData.h"

RoleBasedRouting::RoleBasedRouting()
    : role("") {
}

RoleBasedRouting::RoleBasedRouting(const std::string& roleName)
    : role(roleName) {
}

Participant* RoleBasedRouting::select(const Task& task,
                                     const InstanceData& data,
                                     const ParticipantDirectory& directory) {
    (void)task; // Unused in standard role filtering
    (void)data; // Role routing does not inspect instance payload

    if (role.empty()) {
        return nullptr;
    }

    // Query directory for all candidates qualified with the configured role
    std::vector<Participant*> candidates = directory.withRole(role);
    if (candidates.empty()) {
        return nullptr;
    }

    // Allocate work to the first available matching candidate
    return candidates.front();
}

RoutingStrategy* RoleBasedRouting::clone() const {
    return new RoleBasedRouting(*this);
}

std::string RoleBasedRouting::getRole() const {
    return role;
}

void RoleBasedRouting::setRole(const std::string& roleName) {
    role = roleName;
}
