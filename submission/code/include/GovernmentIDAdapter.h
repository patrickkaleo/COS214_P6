#ifndef GOVERNMENTIDADAPTER_H
#define GOVERNMENTIDADAPTER_H

/**
 * @file GovernmentIDAdapter.h
 * @brief Adapter from ExternalService to the national identity registry (education industry).
 * @author Angela Ramaboea
 * @version 1.0
 */

#include <memory>
#include "ExternalService.h"
#include "NationalIdRegistry.h"

/**
 * @class GovernmentIDAdapter
 * @brief Verifies an applicant's identity for the university-application workflow.
 *
 * Adapter role in the GoF Adapter pattern (object adapter): it owns a NationalIdRegistry
 * (Adaptee) and implements ExternalService (Target).
 *
 * Request parameters: @c idNumber, @c surname.
 * Outputs: @c identityVerified ("true"/"false") and @c identityStatus
 * ("VERIFIED", "SURNAME_MISMATCH", "NOT_FOUND", "DECEASED").
 *
 * A negative answer is still a successful call: the workflow branches on
 * @c identityVerified. Only an invalid request or an offline registry fails.
 *
 * Registered as "IdentityVerificationService", the service name a workflow definition
 * uses for its identity check.
 */
class GovernmentIDAdapter : public ExternalService {
private:
    std::unique_ptr<NationalIdRegistry> registry;

public:
    /**
     * @param adaptee Registry to wrap (ownership taken); a seeded registry is created if nullptr.
     */
    explicit GovernmentIDAdapter(NationalIdRegistry* adaptee = nullptr);

    ServiceResult invoke(const ServiceRequest& request) override;
    std::string name() const override;

    /// Access to the wrapped registry, e.g. to simulate an outage in a demo or test.
    NationalIdRegistry& getRegistry();
};

#endif // GOVERNMENTIDADAPTER_H
