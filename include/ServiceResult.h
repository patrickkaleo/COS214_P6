#ifndef SERVICERESULT_H
#define SERVICERESULT_H

/**
 * @file ServiceResult.h
 * @brief Generic result returned by an external service to the workflow engine.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include <string>
#include <map>
#include "InstanceData.h"

/**
 * @struct ServiceResult
 * @brief Success flag, message and named outputs, translated back from an external API.
 *
 * A failed result (success == false) is the signal for the engine to roll back to the
 * last snapshot or mark the task failed; outputs are written into the instance data.
 */
struct ServiceResult {
    bool success;                               ///< Whether the external call succeeded.
    std::string message;                        ///< Human-readable summary or error.
    std::map<std::string, std::string> outputs; ///< Named outputs to store in InstanceData.

    ServiceResult() : success(false) {}

    /**
     * @brief Creates a successful result.
     * @param msg Summary shown in history.
     * @param out Named outputs.
     */
    static ServiceResult ok(const std::string& msg,
                            const std::map<std::string, std::string>& out = std::map<std::string, std::string>()) {
        ServiceResult r;
        r.success = true;
        r.message = msg;
        r.outputs = out;
        return r;
    }

    /**
     * @brief Creates a failed result.
     * @param msg Error description.
     */
    static ServiceResult failure(const std::string& msg) {
        ServiceResult r;
        r.message = msg;
        return r;
    }

    /**
     * @brief Copies every output into the instance data, overwriting existing keys.
     * @param data The workflow instance's data.
     */
    void applyTo(InstanceData& data) const {
        for (std::map<std::string, std::string>::const_iterator it = outputs.begin(); it != outputs.end(); ++it) {
            data.set(it->first, it->second);
        }
    }
};

#endif // SERVICERESULT_H
