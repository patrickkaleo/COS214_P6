/**
 * @file InstanceData.cpp
 * @brief Implementation of the InstanceData payload dictionary.
 * @author Enock Nyamweya
 * @version 1.0
 */

#include "InstanceData.h"

InstanceData::InstanceData()
    : fields() {
}

InstanceData::InstanceData(const std::map<std::string, std::string>& initialFields)
    : fields(initialFields) {
}

std::string InstanceData::get(const std::string& key) const {
    auto it = fields.find(key);
    if (it != fields.end()) {
        return it->second;
    }
    return "";
}

void InstanceData::set(const std::string& key, const std::string& value) {
    fields[key] = value;
}

bool InstanceData::has(const std::string& key) const {
    return fields.find(key) != fields.end();
}

const std::map<std::string, std::string>& InstanceData::getMap() const {
    return fields;
}
