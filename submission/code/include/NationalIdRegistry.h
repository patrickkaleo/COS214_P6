#ifndef NATIONALIDREGISTRY_H
#define NATIONALIDREGISTRY_H

/**
 * @file NationalIdRegistry.h
 * @brief Adaptee: simulated government identity registry with its own lookup API.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include <string>
#include <map>

/**
 * @class NationalIdRegistry
 * @brief Test stub standing in for a national identity-verification system.
 *
 * Its interface is deliberately unlike ExternalService: it returns integer status codes
 * and an output parameter, and knows nothing about workflows. GovernmentIDAdapter
 * translates between the two. setOnline(false) simulates an outage for exception paths.
 */
class NationalIdRegistry {
public:
    /// Status codes returned by lookupCitizen().
    enum LookupCode {
        FOUND = 0,          ///< Citizen exists; registered surname returned.
        NOT_FOUND = 1,      ///< No citizen with that ID number.
        DECEASED = 2,       ///< Citizen is recorded as deceased.
        INVALID_FORMAT = 3, ///< ID number is not 13 digits.
        UNAVAILABLE = 99    ///< Registry is offline.
    };

    /// Creates a registry seeded with demo citizens.
    NationalIdRegistry();

    /**
     * @brief Adds a citizen record.
     * @param idNumber 13-digit identity number.
     * @param surname Registered surname.
     * @param deceased Whether the citizen is recorded as deceased.
     */
    void addCitizen(const std::string& idNumber, const std::string& surname, bool deceased = false);

    /**
     * @brief Looks up an identity number.
     * @param idNumber Identity number to look up.
     * @param surnameOut Receives the registered surname when FOUND.
     * @return int One of LookupCode.
     */
    int lookupCitizen(const std::string& idNumber, std::string& surnameOut) const;

    /// Simulates the registry going offline (false) or coming back (true).
    void setOnline(bool isOnline);

private:
    struct Record {
        std::string surname;
        bool deceased;
    };
    std::map<std::string, Record> citizens;
    bool online;
};

#endif // NATIONALIDREGISTRY_H
