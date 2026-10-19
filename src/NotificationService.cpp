/**
 * @file NotificationService.cpp
 * @brief Implementation of the NotificationService concrete Observer class.
 * @version 1.0
 */

#include "NotificationService.h"
#include <sstream>

NotificationService::NotificationService(const std::string& channel, bool echo)
    : channelName(channel), echoToConsole(echo) {
}

void NotificationService::onEvent(const WorkflowEvent& e) {
    std::string message = formatNotification(e);
    notificationLog.push_back(message);

    if (echoToConsole) {
        std::cout << message << std::endl;
    }
}

std::string NotificationService::formatNotification(const WorkflowEvent& e) const {
    std::ostringstream oss;
    oss << "[NOTIFICATION][" << channelName << "] ";

    switch (e.type) {
        case EventType::AVAILABLE:
            oss << "Work item '" << e.itemName << "' (" << e.itemId 
                << ") is now AVAILABLE in workflow " << e.instanceId << ".";
            break;
        case EventType::ASSIGNED:
            oss << "Work item '" << e.itemName << "' (" << e.itemId 
                << ") has been ASSIGNED to a participant.";
            break;
        case EventType::STARTED:
            oss << "Work item '" << e.itemName << "' (" << e.itemId 
                << ") has STARTED execution.";
            break;
        case EventType::COMPLETED:
            oss << "Work item '" << e.itemName << "' (" << e.itemId 
                << ") COMPLETED successfully.";
            if (!e.outcome.empty()) {
                oss << " Outcome: " << e.outcome;
            }
            break;
        case EventType::REJECTED:
            oss << "Work item '" << e.itemName << "' (" << e.itemId 
                << ") was REJECTED.";
            if (!e.outcome.empty()) {
                oss << " Reason: " << e.outcome;
            }
            break;
        case EventType::CANCELLED:
            oss << "Work item '" << e.itemName << "' (" << e.itemId 
                << ") has been CANCELLED.";
            break;
        case EventType::ESCALATED:
            oss << "ALERT: Work item '" << e.itemName << "' (" << e.itemId 
                << ") has been ESCALATED due to policy/timeout!";
            if (!e.outcome.empty()) {
                oss << " Details: " << e.outcome;
            }
            break;
        case EventType::FAILED:
            oss << "CRITICAL: Work item '" << e.itemName << "' (" << e.itemId 
                << ") FAILED during execution!";
            if (!e.outcome.empty()) {
                oss << " Error details: " << e.outcome;
            }
            break;
        case EventType::INSTANCE_COMPLETED:
            oss << "Workflow instance " << e.instanceId 
                << " has COMPLETED its full lifecycle.";
            break;
        default:
            oss << "Event " << eventTypeToString(e.type) << " received for item " << e.itemId;
            break;
    }

    return oss.str();
}

const std::vector<std::string>& NotificationService::getNotificationLog() const {
    return notificationLog;
}

size_t NotificationService::getNotificationCount() const {
    return notificationLog.size();
}

void NotificationService::setEchoToConsole(bool echo) {
    echoToConsole = echo;
}

void NotificationService::clearLog() {
    notificationLog.clear();
}
