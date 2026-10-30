#ifndef CARRIERBOOKINGSYSTEM_H
#define CARRIERBOOKINGSYSTEM_H

/**
 * @file CarrierBookingSystem.h
 * @brief Adaptee: simulated legacy carrier booking system with a C-style interface.
 * @author Angela Ramaboea
 * @version 1.0
 */

/**
 * @class CarrierBookingSystem
 * @brief Test stub standing in for a carrier's legacy rebooking system.
 *
 * Uses numeric hub codes, C strings and an output buffer, as legacy integrations often
 * do. CarrierAdapter maps hub names to codes and the return codes to messages.
 */
class CarrierBookingSystem {
public:
    /// Return codes of rebook().
    enum ResultCode {
        OK = 0,
        UNKNOWN_HUB = -1,
        NO_CAPACITY = -2,
        SYSTEM_OFFLINE = -3
    };

    /// Numeric hub codes known to the carrier.
    enum HubCode {
        HUB_JOHANNESBURG = 11,
        HUB_DURBAN = 31,
        HUB_CAPE_TOWN = 21,
        HUB_GQEBERHA = 41
    };

    CarrierBookingSystem();

    /**
     * @brief Books a consignment onto a new route.
     * @param consignmentNo Consignment (waybill) number.
     * @param hubCode Destination HubCode.
     * @param confirmationOut Buffer receiving the confirmation number.
     * @param outLen Size of @p confirmationOut.
     * @return int One of ResultCode.
     */
    int rebook(const char* consignmentNo, int hubCode, char* confirmationOut, int outLen);

    /// Marks a hub as full (true) so rebookings to it fail with NO_CAPACITY.
    void setHubFull(int hubCode, bool full);

    /// Simulates the system going offline (false) or coming back (true).
    void setOnline(bool isOnline);

private:
    bool online;
    int fullHub; ///< Hub currently without capacity, 0 if none.
    int nextConfirmation;
};

#endif // CARRIERBOOKINGSYSTEM_H
