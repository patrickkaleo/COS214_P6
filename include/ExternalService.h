#ifndef EXTERNALSERVICE_H
#define EXTERNALSERVICE_H

/**
 * @file ExternalService.h
 * @brief Adapter Target: the single interface the engine uses for external systems.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include <string>
#include "ServiceRequest.h"
#include "ServiceResult.h"

/**
 * @class ExternalService
 * @brief Target interface of the GoF Adapter (Structural) pattern.
 *
 * Workflows call external applications whose interfaces were not designed for TIVIDY
 * (identity registries, payment gateways, courier systems). The engine depends only on
 * this interface; each concrete adapter translates a ServiceRequest into its adaptee's
 * own API and converts the reply into a ServiceResult.
 *
 * A new integration therefore means writing a new adapter only. Adapters are looked up
 * by name() in the ServiceRegistry, using the service names set by the definition
 * builders (e.g. setService("VERIFY_DOCS", "DocumentVerificationService")).
 *
 * @see GovernmentIDAdapter, BankGatewayAdapter, TrackingServiceAdapter, CarrierAdapter
 */
class ExternalService {
public:
    virtual ~ExternalService() = default;

    /**
     * @brief Performs the external call.
     * @param request Generic request built by the calling task.
     * @return ServiceResult Translated result; success == false on any external failure.
     */
    virtual ServiceResult invoke(const ServiceRequest& request) = 0;

    /**
     * @brief Registry key for this service, matching the name used in workflow definitions.
     * @return std::string The service name.
     */
    virtual std::string name() const = 0;
};

#endif // EXTERNALSERVICE_H
