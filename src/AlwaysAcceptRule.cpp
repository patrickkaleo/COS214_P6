/**
 * @file AlwaysAcceptRule.cpp
 * @brief Unlimited approval authority.
 * @author Angelo Anthony
 * @version 1.0
 */

#include "AlwaysAcceptRule.h"

bool AlwaysAcceptRule::accepts(const ApprovalRequest& request) const {
    (void)request;
    return true;
}

ApprovalRule* AlwaysAcceptRule::clone() const {
    return new AlwaysAcceptRule(*this);
}

std::string AlwaysAcceptRule::describe() const {
    return "any request";
}
