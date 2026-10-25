#ifndef COURIERTRACKINGAPI_H
#define COURIERTRACKINGAPI_H

/**
 * @file CourierTrackingApi.h
 * @brief Adaptee: simulated courier tracking API that answers in a delimited text format.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include <string>
#include <map>

/**
 * @class CourierTrackingApi
 * @brief Test stub standing in for a courier's parcel-tracking web service.
 *
 * queryParcel() returns one line of text:
 *  - @c "OK;<status>;<location>;<delayMinutes>" for a known waybill
 *  - @c "ERR;NOT_FOUND" or @c "ERR;SERVICE_DOWN" otherwise.
 *
 * TrackingServiceAdapter parses this format into named workflow data.
 */
class CourierTrackingApi {
public:
    /// Creates the API seeded with demo parcels.
    CourierTrackingApi();

    /**
     * @brief Adds or replaces a parcel's tracking record.
     * @param waybill Waybill number.
     * @param status Courier status code, e.g. "IN_TRANSIT", "DELAYED", "DAMAGED".
     * @param location Last scan location.
     * @param delayMinutes Minutes behind the promised delivery time.
     */
    void registerParcel(const std::string& waybill, const std::string& status,
                        const std::string& location, int delayMinutes);

    /// @return The delimited tracking line for @p waybill.
    std::string queryParcel(const std::string& waybill) const;

    /// Simulates the API going offline (false) or coming back (true).
    void setOnline(bool isOnline);

private:
    std::map<std::string, std::string> parcels; ///< waybill -> "status;location;delayMinutes"
    bool online;
};

#endif // COURIERTRACKINGAPI_H
