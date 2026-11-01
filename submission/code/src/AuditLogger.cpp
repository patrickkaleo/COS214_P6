/**
 * @file AuditLogger.cpp
 * @brief Implementation of the AuditLogger concrete Observer class.
 * @author Enock Nyamweya
 * @version 2.0
 */

#include "AuditLogger.h"
#include <sstream>
#include <iomanip>
#include <ctime>

AuditLogger::AuditLogger(bool echo)
    : echoToConsole(echo) {
}

void AuditLogger::onEvent(const WorkflowEvent& e) {
    std::string entry = formatAuditEntry(e);
    auditTrail.push_back(entry);

    if (echoToConsole) {
        std::cout << entry << std::endl;
    }
}

std::string AuditLogger::formatAuditEntry(const WorkflowEvent& e) const {
    char timeBuffer[64];
    struct tm* tmInfo = std::localtime(&e.timestamp);
    if (tmInfo) {
        std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", tmInfo);
    } else {
        std::snprintf(timeBuffer, sizeof(timeBuffer), "timestamp:%ld", static_cast<long>(e.timestamp));
    }

    std::ostringstream oss;
    oss << "[AUDIT] [" << timeBuffer << "]"
        << " [INSTANCE:" << e.instanceId << "]"
        << " [ITEM:" << e.itemId << " (" << e.itemName << ")]"
        << " [TRANSITION:" << eventTypeToString(e.type) << "]";

    if (!e.outcome.empty()) {
        oss << " [OUTCOME:" << e.outcome << "]";
    }

    return oss.str();
}

const std::vector<std::string>& AuditLogger::getAuditTrail() const {
    return auditTrail;
}

size_t AuditLogger::getEntryCount() const {
    return auditTrail.size();
}

void AuditLogger::printAuditTrail() const {
    for (const auto& entry : auditTrail) {
        std::cout << entry << "\n";
    }
}

void AuditLogger::setEchoToConsole(bool echo) {
    echoToConsole = echo;
}

void AuditLogger::clearLog() {
    auditTrail.clear();
}
