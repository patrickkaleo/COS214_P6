/**
 * @file TrackingServiceAdapter.cpp
 * @brief Parses courier tracking replies into workflow data.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "TrackingServiceAdapter.h"
#include <cstdlib>
#include <sstream>
#include <vector>

namespace {

std::vector<std::string> split(const std::string& line, char sep) {
    std::vector<std::string> parts;
    std::istringstream in(line);
    std::string part;
    while (std::getline(in, part, sep)) {
        parts.push_back(part);
    }
    return parts;
}

} // namespace

TrackingServiceAdapter::TrackingServiceAdapter(CourierTrackingApi* adaptee)
    : api(adaptee != nullptr ? adaptee : new CourierTrackingApi()) {
}

ServiceResult TrackingServiceAdapter::invoke(const ServiceRequest& request) {
    std::string waybill = request.get("waybill");
    if (waybill.empty()) {
        return ServiceResult::failure("Tracking lookup needs a 'waybill'");
    }

    std::vector<std::string> fields = split(api->queryParcel(waybill), ';');

    if (fields.size() == 2 && fields[0] == "ERR") {
        if (fields[1] == "NOT_FOUND") {
            return ServiceResult::failure("Unknown waybill " + waybill);
        }
        return ServiceResult::failure("Courier tracking service unavailable");
    }
    if (fields.size() != 4 || fields[0] != "OK") {
        return ServiceResult::failure("Unexpected reply from courier tracking service");
    }

    int delayMinutes = std::atoi(fields[3].c_str());
    std::ostringstream hours;
    hours << (delayMinutes + 59) / 60;

    std::map<std::string, std::string> out;
    out["shipmentStatus"] = fields[1];
    out["lastLocation"] = fields[2];
    out["delayHours"] = hours.str();
    return ServiceResult::ok("Shipment " + waybill + " is " + fields[1], out);
}

std::string TrackingServiceAdapter::name() const {
    return "TrackingService";
}

CourierTrackingApi& TrackingServiceAdapter::getApi() {
    return *api;
}
