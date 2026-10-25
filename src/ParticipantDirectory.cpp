/**
 * @file ParticipantDirectory.cpp
 * @brief Implementation of the ParticipantDirectory manager.
 * @author Enock Nyamweya
 * @version 2.0
 */

#include "ParticipantDirectory.h"
#include "Participant.h"

ParticipantDirectory::ParticipantDirectory()
    : participants() {
}

void ParticipantDirectory::addParticipant(std::shared_ptr<Participant> p) {
    if (p) {
        participants.push_back(p);
    }
}

void ParticipantDirectory::addParticipant(Participant* p) {
    if (p) {
        participants.push_back(std::shared_ptr<Participant>(p));
    }
}

std::vector<std::shared_ptr<Participant>> ParticipantDirectory::withRole(const std::string& roleName) const {
    std::vector<std::shared_ptr<Participant>> matches;
    for (const auto& p : participants) {
        if (p && p->hasRole(roleName)) {
            matches.push_back(p);
        }
    }
    return matches;
}

std::shared_ptr<Participant> ParticipantDirectory::findById(const std::string& id) const {
    for (const auto& p : participants) {
        if (p && p->getId() == id) {
            return p;
        }
    }
    return nullptr;
}

size_t ParticipantDirectory::size() const {
    return participants.size();
}

void ParticipantDirectory::clear() {
    participants.clear();
}
