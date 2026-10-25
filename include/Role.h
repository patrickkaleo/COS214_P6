#ifndef ROLE_H
#define ROLE_H

/**
 * @file Role.h
 * @brief Value object representing an organizational qualification or role.
 * @author Enock Nyamweya
 * @version 1.0
 */

#include <string>

class Role {
private:
    std::string name;

public:
    Role();
    explicit Role(const std::string& roleName);

    std::string getName() const;
    void setName(const std::string& roleName);

    bool operator==(const Role& other) const;
    bool operator!=(const Role& other) const;
};

#endif // ROLE_H
