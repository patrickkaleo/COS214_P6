#ifndef PARTICIPANT_H
#define PARTICIPANT_H

/**
 * @file Participant.h
 * @brief Domain entity representing an organizational worker or service actor.
 * @author Enock Nyamweya
 * @version 1.0
 */

#include <string>
#include <vector>
#include "Role.h"

class Participant {
private:
    std::string participantID;
    std::string name;
    std::vector<Role> roles;
    int workload;

public:
    Participant();
    Participant(const std::string& id,
                const std::string& participantName,
                const std::string& initialRole = "",
                int initialWorkload = 0);

    std::string getId() const;
    std::string getName() const;
    int getWorkload() const;
    void setWorkload(int load);

    void addRole(const std::string& roleName);
    void addRole(const Role& role);
    bool hasRole(const std::string& roleName) const;
    const std::vector<Role>& getRoles() const;
};

#endif // PARTICIPANT_H
