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

class InstanceData {
private:
    std::map<std::string, std::string> fields;

public:
    InstanceData();
    InstanceData(const std::map<std::string, std::string>& initialFields);

    std::string get(const std::string& key) const;
    void set(const std::string& key, const std::string& value);
    bool has(const std::string& key) const;
    const std::map<std::string, std::string>& getMap() const;
};

#endif // INSTANCEDATA_H
