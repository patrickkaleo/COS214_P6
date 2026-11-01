#ifndef INSTANCEDATA_H
#define INSTANCEDATA_H

/**
 * @file InstanceData.h
 * @brief Key-value process payload container for workflow instance runtime attributes.
 * @author Enock Nyamweya
 * @version 1.0
 */

#include <string>
#include <map>

/** @brief A container for storing key-value pairs of workflow instance runtime attributes. */
class InstanceData
{
private:
    std::map<std::string, std::string> fields;

public:
    /** @brief Constructs a new InstanceData instance.
     */
    InstanceData();
    /** @brief Constructs a new InstanceData instance with initial fields.
     * @param initialFields The initial key-value pairs for the instance data.
     */
    InstanceData(const std::map<std::string, std::string> &initialFields);

    /** @brief Retrieves the value associated with the specified key.
     * @param key The key for which to retrieve the value.
     * @return The value associated with the key, or an empty string if the key is not found.
     */
    std::string get(const std::string &key) const;
    /** @brief Sets the value associated with the specified key.
     * @param key The key for which to set the value.
     * @param value The value to associate with the key.
     */
    void set(const std::string &key, const std::string &value);
    /** @brief Checks if the instance data contains the specified key.
     * @param key The key to check.
     * @return true if the key is found, false otherwise.
     */
    bool has(const std::string &key) const;
    /** @brief Returns the underlying map of key-value pairs.
     * @return A constant reference to the map of key-value pairs.
     */
    const std::map<std::string, std::string> &getMap() const;
};

#endif // INSTANCEDATA_H
