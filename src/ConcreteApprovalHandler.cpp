/**
 * @file ConcreteApprovalHandler.cpp
 * @brief Level skipping, authority check and approver lookup for one approval level.
 * @author Angelo Anthony
 * @version 1.0
 */

#include "ConcreteApprovalHandler.h"
#include <vector>
#include <iostream>
#include "ApprovalRequest.h"
#include "ApprovalRule.h"
#include "ParticipantDirectory.h"
#include "Participant.h"

ConcreteApprovalHandler::ConcreteApprovalHandler(const std::string& role,
                                                 ApprovalRule* authority,
                                                 int deadline)
    : approverRole(role),
      rule(authority),
      deadlineHours(deadline) {
}

ConcreteApprovalHandler::~ConcreteApprovalHandler() = default;

Participant* ConcreteApprovalHandler::handle(ApprovalRequest& request) {
    if (request.level > 0) {
        // The task was escalated past this level (e.g. its deadline passed here).
        ApprovalRequest escalated(request);
        --escalated.level;
        return passToNext(escalated);
    }

    if (rule && rule->accepts(request) && request.directory != nullptr) {
        auto approvers = request.directory->withRole(approverRole);
        if (!approvers.empty()) {
            return approvers.front().get();
        }
    }
    return passToNext(request);
}

ApprovalHandler* ConcreteApprovalHandler::clone() const {
    ApprovalRule* ruleCopy = rule ? rule->clone() : nullptr;
    return withClonedNext(new ConcreteApprovalHandler(approverRole, ruleCopy, deadlineHours));
}

std::string ConcreteApprovalHandler::describe() const {
    return approverRole + " (" + (rule ? rule->describe() : "no authority") + ")";
}

std::string ConcreteApprovalHandler::getApproverRole() const {
    return approverRole;
}

int ConcreteApprovalHandler::getDeadlineHours() const {
    return deadlineHours;
}
