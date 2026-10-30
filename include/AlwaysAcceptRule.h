#ifndef ALWAYSACCEPTRULE_H
#define ALWAYSACCEPTRULE_H

/**
 * @file AlwaysAcceptRule.h
 * @brief Approval rule with unlimited authority, used for the last level of a chain.
 * @author Angelo Anthony
 * @version 1.0
 */

#include "ApprovalRule.h"

/**
 * @class AlwaysAcceptRule
 * @brief Accepts every request, e.g. for a committee that can approve any amount.
 */
class AlwaysAcceptRule : public ApprovalRule {
public:
    bool accepts(const ApprovalRequest& request) const override;
    ApprovalRule* clone() const override;
    std::string describe() const override;
};

#endif // ALWAYSACCEPTRULE_H
