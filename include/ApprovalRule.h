#ifndef APPROVALRULE_H
#define APPROVALRULE_H

/**
 * @file ApprovalRule.h
 * @brief Abstract rule deciding whether an approval level may decide a request.
 * @author Angelo Anthony
 * @version 1.0
 */

#include <string>

class ApprovalRequest;

/**
 * @class ApprovalRule
 * @brief Limit of one approval level's authority, e.g. "amount up to R20 000".
 *
 * Each ConcreteApprovalHandler owns one rule. Keeping the limit in a separate object
 * lets one handler class serve every kind of limit.
 *
 * @see ThresholdRule, AlwaysAcceptRule
 */
class ApprovalRule {
public:
    virtual ~ApprovalRule() = default;

    /**
     * @param request The approval being asked for.
     * @return true if this level has the authority to decide it.
     */
    virtual bool accepts(const ApprovalRequest& request) const = 0;

    /// @return Newly allocated copy of this rule.
    virtual ApprovalRule* clone() const = 0;

    /// @return Short description, e.g. "amount <= 20000".
    virtual std::string describe() const = 0;
};

#endif // APPROVALRULE_H
