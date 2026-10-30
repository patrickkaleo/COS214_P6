/**
 * @file NationalIdRegistry.cpp
 * @brief Simulated national identity registry used by GovernmentIDAdapter.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "NationalIdRegistry.h"
#include <cctype>

NationalIdRegistry::NationalIdRegistry()
    : online(true) {
    addCitizen("0501015800087", "Mokoena");
    addCitizen("0403120123081", "Naidoo");
    addCitizen("0607254455089", "van der Merwe");
    addCitizen("4808105009083", "Dlamini", true);
}

void NationalIdRegistry::addCitizen(const std::string& idNumber, const std::string& surname, bool deceased) {
    Record record;
    record.surname = surname;
    record.deceased = deceased;
    citizens[idNumber] = record;
}

int NationalIdRegistry::lookupCitizen(const std::string& idNumber, std::string& surnameOut) const {
    if (!online) {
        return UNAVAILABLE;
    }

    if (idNumber.size() != 13) {
        return INVALID_FORMAT;
    }
    for (std::string::size_type i = 0; i < idNumber.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(idNumber[i]))) {
            return INVALID_FORMAT;
        }
    }

    std::map<std::string, Record>::const_iterator it = citizens.find(idNumber);
    if (it == citizens.end()) {
        return NOT_FOUND;
    }

    surnameOut = it->second.surname;
    return it->second.deceased ? DECEASED : FOUND;
}

void NationalIdRegistry::setOnline(bool isOnline) {
    online = isOnline;
}
