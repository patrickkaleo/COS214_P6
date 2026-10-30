#ifndef CARRIERADAPTER_H
#define CARRIERADAPTER_H

/**
 * @file CarrierAdapter.h
 * @brief Adapter from ExternalService to the legacy carrier booking system (logistics industry).
 * @author Angela Ramaboea
 * @version 1.0
 */

#include <map>
#include <memory>
#include "ExternalService.h"
#include "CarrierBookingSystem.h"

/**
 * @class CarrierAdapter
 * @brief Rebooks a shipment onto a new route for the REROUTE task.
 *
 * Adapter role in the GoF Adapter pattern: wraps CarrierBookingSystem (Adaptee). It
 * maps destination hub names to the carrier's numeric codes, manages the C output
 * buffer and translates return codes into ServiceResults.
 *
 * Request parameters: @c waybill, @c destinationHub (e.g. "Cape Town").
 * Outputs on success: @c rerouteReference, @c newHub.
 *
 * Registered as "CarrierAdapter", the name LogisticsDefinitionBuilder sets on REROUTE.
 */
class CarrierAdapter : public ExternalService {
private:
    std::unique_ptr<CarrierBookingSystem> carrier;
    std::map<std::string, int> hubCodes; ///< Hub name -> CarrierBookingSystem::HubCode.

public:
    /// @param adaptee Booking system to wrap (ownership taken); one is created if nullptr.
    explicit CarrierAdapter(CarrierBookingSystem* adaptee = nullptr);

    ServiceResult invoke(const ServiceRequest& request) override;
    std::string name() const override;

    /// Access to the wrapped booking system, e.g. to simulate a full hub or an outage.
    CarrierBookingSystem& getCarrier();
};

#endif // CARRIERADAPTER_H
