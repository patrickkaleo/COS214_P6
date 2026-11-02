#ifndef SERVICEREQUEST_H
#define SERVICEREQUEST_H

/**
 * @file ServiceRequest.h
 * @brief Generic request passed from a task to an external service.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include <string>
#include <map>

/**
 * @struct ServiceRequest
 * @brief Operation name plus named string parameters, independent of any external API.
 *
 * Tasks build a request from their instance data, so the engine never needs to know
 * the parameter types or call conventions of a particular external system.
 */
struct ServiceRequest {
    std::string operation;                         ///< What the caller wants, e.g. "verifyIdentity".
    std::map<std::string, std::string> parameters; ///< Named inputs, usually copied from InstanceData.

    ServiceRequest() = default;

    /**
     * @param op Operation name.
     * @param params Named inputs (e.g. InstanceData::getMap()).
     */
    ServiceRequest(const std::string& op, const std::map<std::string, std::string>& params)
        : operation(op), parameters(params) {}

    /**
     * @brief Looks up a parameter.
     * @param key Parameter name.
     * @return std::string The value, or an empty string if the parameter is missing.
     */
    std::string get(const std::string& key) const {
        std::map<std::string, std::string>::const_iterator it = parameters.find(key);
        return it == parameters.end() ? "" : it->second;
    }
};

#endif // SERVICEREQUEST_H
