/**
 * @file ParticipantDirectory.cpp
 * @brief Implementation of the ParticipantDirectory manager.
 * @author Enock Nyamweya
 * @version 1.0
 */

#include "ParticipantDirectory.h"
#include "Participant.h"

ParticipantDirectory::ParticipantDirectory()
    : participants() {
}

ParticipantDirectory::~ParticipantDirectory() {
    clear();
}

void ParticipantDirectory::addParticipant(Participant* p) {
    if (p) {
        participants.push_back(p);
    }
}

std::vector<Participant*> ParticipantDirectory::withRole(const std::string& roleName) const {
    std::vector<Participant*> matches;
    for (Participant* p : participants) {
        if (p && p->hasRole(roleName)) {
            matches.push_back(p);
        }
    }
    return matches;
}

Participant* ParticipantDirectory::findById(const std::string& id) const {
    for (Participant* p : participants) {
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
    for (Participant* p : participants) {
        delete p;
    }
    participants.clear();
}
