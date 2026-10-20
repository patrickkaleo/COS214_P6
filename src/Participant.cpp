/**
 * @file Participant.cpp
 * @brief Implementation of the Participant domain entity.
 * @author Enock Nyamweya
 * @version 1.0
 */

#include "Participant.h"

Participant::Participant()
    : participantID(""), name(""), roles(), workload(0) {
}

Participant::Participant(const std::string& id,
                         const std::string& participantName,
                         const std::string& initialRole,
                         int initialWorkload)
    : participantID(id), name(participantName), roles(), workload(initialWorkload) {
    if (!initialRole.empty()) {
        roles.push_back(Role(initialRole));
    }
}

std::string Participant::getId() const {
    return participantID;
}

std::string Participant::getName() const {
    return name;
}

int Participant::getWorkload() const {
    return workload;
}

void Participant::setWorkload(int load) {
    workload = load;
}

void Participant::addRole(const std::string& roleName) {
    roles.push_back(Role(roleName));
}

void Participant::addRole(const Role& role) {
    roles.push_back(role);
}

bool Participant::hasRole(const std::string& roleName) const {
    for (const Role& r : roles) {
        if (r.getName() == roleName) {
            return true;
        }
    }
    return false;
}

const std::vector<Role>& Participant::getRoles() const {
    return roles;
}
