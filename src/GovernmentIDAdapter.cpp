/**
 * @file GovernmentIDAdapter.cpp
 * @brief Translates identity-verification requests into NationalIdRegistry lookups.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "GovernmentIDAdapter.h"
#include <algorithm>
#include <cctype>

namespace {

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

ServiceResult identityResult(bool verified, const std::string& status, const std::string& msg) {
    std::map<std::string, std::string> out;
    out["identityVerified"] = verified ? "true" : "false";
    out["identityStatus"] = status;
    return ServiceResult::ok(msg, out);
}

} // namespace

GovernmentIDAdapter::GovernmentIDAdapter(NationalIdRegistry* adaptee)
    : registry(adaptee != nullptr ? adaptee : new NationalIdRegistry()) {
}

ServiceResult GovernmentIDAdapter::invoke(const ServiceRequest& request) {
    std::string idNumber = request.get("idNumber");
    std::string surname = request.get("surname");
    if (idNumber.empty() || surname.empty()) {
        return ServiceResult::failure("Identity check needs 'idNumber' and 'surname'");
    }

    std::string registeredSurname;
    int code = registry->lookupCitizen(idNumber, registeredSurname);

    // Registry status codes -> engine result
    switch (code) {
        case NationalIdRegistry::FOUND:
            if (toLower(registeredSurname) == toLower(surname)) {
                return identityResult(true, "VERIFIED", "Identity verified");
            }
            return identityResult(false, "SURNAME_MISMATCH", "Surname does not match the identity record");
        case NationalIdRegistry::NOT_FOUND:
            return identityResult(false, "NOT_FOUND", "No identity record for this ID number");
        case NationalIdRegistry::DECEASED:
            return identityResult(false, "DECEASED", "Identity record is marked as deceased");
        case NationalIdRegistry::INVALID_FORMAT:
            return ServiceResult::failure("ID number must be 13 digits");
        default:
            return ServiceResult::failure("Identity registry unavailable");
    }
}

std::string GovernmentIDAdapter::name() const {
    return "DocumentVerificationService";
}

NationalIdRegistry& GovernmentIDAdapter::getRegistry() {
    return *registry;
}
