/**
 * @file Role.cpp
 * @brief Implementation of the Role value object.
 * @author Enock Nyamweya
 * @version 1.0
 */

#include "Role.h"

Role::Role()
    : name("") {
}

Role::Role(const std::string& roleName)
    : name(roleName) {
}

std::string Role::getName() const {
    return name;
}

void Role::setName(const std::string& roleName) {
    name = roleName;
}

bool Role::operator==(const Role& other) const {
    return name == other.name;
}

bool Role::operator!=(const Role& other) const {
    return !(*this == other);
}
