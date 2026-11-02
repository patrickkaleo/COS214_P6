/**
 * @file CarrierBookingSystem.cpp
 * @brief Simulated legacy carrier booking system used by CarrierAdapter.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "CarrierBookingSystem.h"
#include <cstdio>

CarrierBookingSystem::CarrierBookingSystem()
    : online(true), fullHub(0), nextConfirmation(1) {
}

int CarrierBookingSystem::rebook(const char* consignmentNo, int hubCode, char* confirmationOut, int outLen) {
    if (!online) {
        return SYSTEM_OFFLINE;
    }
    if (hubCode != HUB_JOHANNESBURG && hubCode != HUB_DURBAN &&
        hubCode != HUB_CAPE_TOWN && hubCode != HUB_GQEBERHA) {
        return UNKNOWN_HUB;
    }
    if (hubCode == fullHub) {
        return NO_CAPACITY;
    }

    std::snprintf(confirmationOut, static_cast<size_t>(outLen), "RB%02d-%s-%03d",
                  hubCode, consignmentNo, nextConfirmation++);
    return OK;
}

void CarrierBookingSystem::setHubFull(int hubCode, bool full) {
    fullHub = full ? hubCode : 0;
}

void CarrierBookingSystem::setOnline(bool isOnline) {
    online = isOnline;
}
