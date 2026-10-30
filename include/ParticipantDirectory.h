#ifndef PARTICIPANTDIRECTORY_H
#define PARTICIPANTDIRECTORY_H

/**
 * @file ParticipantDirectory.h
 * @brief Roster managing organizational participants and role-based query services.
 * @author Enock Nyamweya
 * @version 2.0
 */

#include <string>
#include <vector>
#include <memory>

class Participant;

/**
 * @class ParticipantDirectory
 * @brief Roster managing active participants and supporting role-based lookups.
 *
 * Implements shared ownership via std::shared_ptr<Participant> so participants
 * can be safely referenced across tasks, workflow definitions, and routing strategies
 * without dangling pointer risks or manual deallocation errors.
 */
class ParticipantDirectory {
private:
    std::vector<std::shared_ptr<Participant>> participants; ///< Owned participants roster.

public:
    ParticipantDirectory();
    ~ParticipantDirectory() = default;

    // Prevent accidental copying to enforce clear roster ownership
    ParticipantDirectory(const ParticipantDirectory&) = delete;
    ParticipantDirectory& operator=(const ParticipantDirectory&) = delete;

    /**
     * @brief Adds a participant to the directory via shared ownership.
     * @param p Shared pointer to the participant.
     */
    void addParticipant(std::shared_ptr<Participant> p);

    /**
     * @brief Backward-compatible overload accepting raw pointer and taking ownership.
     * @param p Pointer to the newly allocated participant.
     */
    void addParticipant(Participant* p);

    /**
     * @brief Queries participants possessing a given qualification role.
     * @param roleName The name of the role to search for.
     * @return std::vector<std::shared_ptr<Participant>> List of qualified participants.
     */
    std::vector<std::shared_ptr<Participant>> withRole(const std::string& roleName) const;

    /**
     * @brief Finds a participant by unique identifier.
     * @param id The participant's identifier.
     * @return std::shared_ptr<Participant> Matching participant or nullptr.
     */
    std::shared_ptr<Participant> findById(const std::string& id) const;

    size_t size() const;
    void clear();
};

#endif // PARTICIPANTDIRECTORY_H
