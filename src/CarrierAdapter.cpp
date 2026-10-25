/**
 * @file CarrierAdapter.cpp
 * @brief Translates reroute requests into CarrierBookingSystem rebook calls.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "CarrierAdapter.h"

CarrierAdapter::CarrierAdapter(CarrierBookingSystem* adaptee)
    : carrier(adaptee != nullptr ? adaptee : new CarrierBookingSystem()) {
    hubCodes["Johannesburg"] = CarrierBookingSystem::HUB_JOHANNESBURG;
    hubCodes["Durban"] = CarrierBookingSystem::HUB_DURBAN;
    hubCodes["Cape Town"] = CarrierBookingSystem::HUB_CAPE_TOWN;
    hubCodes["Gqeberha"] = CarrierBookingSystem::HUB_GQEBERHA;
}

ServiceResult CarrierAdapter::invoke(const ServiceRequest& request) {
    std::string waybill = request.get("waybill");
    std::string hub = request.get("destinationHub");
    if (waybill.empty() || hub.empty()) {
        return ServiceResult::failure("Reroute needs 'waybill' and 'destinationHub'");
    }

    std::map<std::string, int>::const_iterator code = hubCodes.find(hub);
    if (code == hubCodes.end()) {
        return ServiceResult::failure("Carrier does not serve hub '" + hub + "'");
    }

    char confirmation[64] = {0};
    int rc = carrier->rebook(waybill.c_str(), code->second, confirmation, sizeof(confirmation));

    switch (rc) {
        case CarrierBookingSystem::OK: {
            std::map<std::string, std::string> out;
            out["rerouteReference"] = confirmation;
            out["newHub"] = hub;
            return ServiceResult::ok("Shipment " + waybill + " rerouted to " + hub, out);
        }
        case CarrierBookingSystem::NO_CAPACITY:
            return ServiceResult::failure("No carrier capacity at " + hub);
        case CarrierBookingSystem::UNKNOWN_HUB:
            return ServiceResult::failure("Carrier rejected hub '" + hub + "'");
        default:
            return ServiceResult::failure("Carrier booking system unavailable");
    }
}

std::string CarrierAdapter::name() const {
    return "CarrierAdapter";
}

CarrierBookingSystem& CarrierAdapter::getCarrier() {
    return *carrier;
}
