/**
 * @file RoleBasedRouting.cpp
 * @brief Implementation of role-based task allocation strategy.
 * @author Enock Nyamweya
 * @version 2.0
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

std::shared_ptr<Participant> RoleBasedRouting::select(const Task& task,
                                                     const InstanceData& data,
                                                     const ParticipantDirectory& directory) {
    (void)task; // Unused in standard role filtering
    (void)data; // Role routing does not inspect instance payload

    if (role.empty()) {
        return nullptr;
    }

    // Query directory for all candidates qualified with the configured role
    std::vector<std::shared_ptr<Participant>> candidates = directory.withRole(role);
    if (candidates.empty()) {
        return nullptr;
    }

    // Allocate work to the first available matching candidate
    return candidates.front();
}

std::unique_ptr<RoutingStrategy> RoleBasedRouting::clone() const {
    return std::unique_ptr<RoutingStrategy>(new RoleBasedRouting(*this));
}

std::string RoleBasedRouting::getRole() const {
    return role;
}

void RoleBasedRouting::setRole(const std::string& roleName) {
    role = roleName;
}
