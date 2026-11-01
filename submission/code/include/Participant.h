#ifndef PARTICIPANT_H
#define PARTICIPANT_H

/**
 * @file Participant.h
 * @brief Domain entity representing an organizational worker or service actor.
 * @author Enock Nyamweya
 * @version 2.0
 */

#include <string>
#include <vector>
#include "Role.h"

class Participant
{
private:
    std::string participantID;
    std::string name;
    std::vector<Role> roles;
    int workload;

public:
    /** @brief Constructs a new Participant instance. */
    Participant();
    /** @brief Constructs a new Participant instance with initial attributes.
     * @param id The unique identifier for the participant.
     * @param participantName The name of the participant.
     * @param initialRole The initial role for the participant.
     * @param initialWorkload The initial workload for the participant.
     */
    Participant(const std::string &id,
                const std::string &participantName,
                const std::string &initialRole = "",
                int initialWorkload = 0);
    /** @brief Returns the unique identifier for the participant.
     * @return The participant ID.
     */
    std::string getId() const;
    /** @brief Returns the name of the participant.
     * @return The participant name.
     */
    std::string getName() const;
    /** @brief Returns the workload for the participant.
     * @return The workload.
     */
    int getWorkload() const;
    /** @brief Sets the workload for the participant.
     * @param load The new workload.
     */
    void setWorkload(int load);
    /** @brief Adds a role to the participant.
     * @param roleName The name of the role to add.
     */
    void addRole(const std::string &roleName);
    /** @brief Adds a role to the participant.
     * @param role The role to add.
     */
    void addRole(const Role &role);
    /** @brief Checks if the participant has the specified role.
     * @param roleName The name of the role to check.
     * @return true if the participant has the role, false otherwise.
     */
    bool hasRole(const std::string &roleName) const;
    /** @brief Returns the roles assigned to the participant.
     * @return A constant reference to the vector of roles.
     */
    const std::vector<Role> &getRoles() const;
};

#endif // PARTICIPANT_H
