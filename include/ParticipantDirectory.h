#ifndef PARTICIPANTDIRECTORY_H
#define PARTICIPANTDIRECTORY_H

/**
 * @file ParticipantDirectory.h
 * @brief Roster managing organizational participants and role-based query services.
 * @author Enock Nyamweya
 * @version 1.0
 */

#include <string>
#include <vector>

class Participant;

class ParticipantDirectory {
private:
    std::vector<Participant*> participants; ///< Owned participants roster.

public:
    ParticipantDirectory();
    ~ParticipantDirectory();

    // Prevent accidental copying to enforce strict single ownership of participant memory
    ParticipantDirectory(const ParticipantDirectory&) = delete;
    ParticipantDirectory& operator=(const ParticipantDirectory&) = delete;

    void addParticipant(Participant* p);
    std::vector<Participant*> withRole(const std::string& roleName) const;
    Participant* findById(const std::string& id) const;
    size_t size() const;
    void clear();
};

#endif // PARTICIPANTDIRECTORY_H
