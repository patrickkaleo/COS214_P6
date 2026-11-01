#ifndef ROLE_H
#define ROLE_H

/**
 * @file Role.h
 * @brief Value object representing an organizational qualification or role.
 * @author Enock Nyamweya
 * @version 2.0
 */

#include <string>

/** @brief The Role class represents an organizational qualification or role.
 *
 * This class encapsulates the name of a role and provides methods for comparison and retrieval.
 */
class Role
{
private:
    std::string name;

public:
    /** @brief Constructs a new Role instance.
     */
    Role();
    /** @brief Constructs a new Role instance with the specified name.
     * @param roleName The name of the role.
     */
    explicit Role(const std::string &roleName);
    /** @brief Returns the name of the role.
     * @return The role name.
     */
    std::string getName() const;
    /** @brief Sets the name of the role.
     * @param roleName The new name of the role.
     */
    void setName(const std::string &roleName);

    /** @brief Compares two Role instances for equality.
     * @param other The other Role instance to compare with.
     * @return true if the roles are equal, false otherwise.
     */
    bool operator==(const Role &other) const;
    /** @brief Compares two Role instances for inequality.
     * @param other The other Role instance to compare with.
     * @return true if the roles are not equal, false otherwise.
     */
    bool operator!=(const Role &other) const;
};

#endif // ROLE_H
