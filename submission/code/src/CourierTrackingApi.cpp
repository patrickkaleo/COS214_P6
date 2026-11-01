/**
 * @file CourierTrackingApi.cpp
 * @brief Simulated courier tracking API used by TrackingServiceAdapter.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "CourierTrackingApi.h"
#include <sstream>

CourierTrackingApi::CourierTrackingApi()
    : online(true) {
    registerParcel("WB1001", "IN_TRANSIT", "Durban Hub", 0);
    registerParcel("WB1002", "DELAYED", "Bloemfontein Depot", 540);
    registerParcel("WB1003", "DAMAGED", "Johannesburg Hub", 1440);
}

void CourierTrackingApi::registerParcel(const std::string& waybill, const std::string& status,
                                        const std::string& location, int delayMinutes) {
    std::ostringstream record;
    record << status << ";" << location << ";" << delayMinutes;
    parcels[waybill] = record.str();
}

std::string CourierTrackingApi::queryParcel(const std::string& waybill) const {
    if (!online) {
        return "ERR;SERVICE_DOWN";
    }
    std::map<std::string, std::string>::const_iterator it = parcels.find(waybill);
    if (it == parcels.end()) {
        return "ERR;NOT_FOUND";
    }
    return "OK;" + it->second;
}

void CourierTrackingApi::setOnline(bool isOnline) {
    online = isOnline;
}
