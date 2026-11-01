#ifndef TRACKINGSERVICEADAPTER_H
#define TRACKINGSERVICEADAPTER_H

/**
 * @file TrackingServiceAdapter.h
 * @brief Adapter from ExternalService to the courier tracking API (logistics industry).
 * @author Angela Ramaboea
 * @version 1.0
 */

#include <memory>
#include "ExternalService.h"
#include "CourierTrackingApi.h"

/**
 * @class TrackingServiceAdapter
 * @brief Fetches a shipment's tracking details for the shipment-exception workflow.
 *
 * Adapter role in the GoF Adapter pattern: wraps CourierTrackingApi (Adaptee),
 * parses its "OK;status;location;delayMinutes" reply and converts the delay to hours.
 *
 * Request parameters: @c waybill.
 * Outputs: @c shipmentStatus, @c lastLocation, @c delayHours (rounded up).
 *
 * Registered as "TrackingService", the name LogisticsDefinitionBuilder sets on CLASSIFY.
 */
class TrackingServiceAdapter : public ExternalService {
private:
    std::unique_ptr<CourierTrackingApi> api;

public:
    /// @param adaptee API to wrap (ownership taken); a seeded API is created if nullptr.
    explicit TrackingServiceAdapter(CourierTrackingApi* adaptee = nullptr);

    ServiceResult invoke(const ServiceRequest& request) override;
    std::string name() const override;

    /// Access to the wrapped API, e.g. to register parcels or simulate an outage.
    CourierTrackingApi& getApi();
};

#endif // TRACKINGSERVICEADAPTER_H
