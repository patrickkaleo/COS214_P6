#ifndef THRESHOLDRULE_H
#define THRESHOLDRULE_H

/**
 * @file ThresholdRule.h
 * @brief Approval rule that accepts requests whose numeric field is within a limit.
 * @author Angelo Anthony
 * @version 1.0
 */

#include <string>
#include "ApprovalRule.h"

/**
 * @class ThresholdRule
 * @brief Accepts a request when instance data field @c field holds a number <= @c maxValue.
 *
 * The field name and limit are definition data, e.g. a bursary "amount" of up to
 * 20000 for an officer, or a shipment "value" for a supervisor. A missing or
 * non-numeric field is not accepted, so the request moves up the chain.
 */
class ThresholdRule : public ApprovalRule {
private:
    std::string field; ///< Instance data key holding the value to check.
    double maxValue;   ///< Largest value this level may approve.

public:
    ThresholdRule(const std::string& fieldName, double limit);

    bool accepts(const ApprovalRequest& request) const override;
    ApprovalRule* clone() const override;
    std::string describe() const override;

    std::string getField() const;
    double getMaxValue() const;
};

#endif // THRESHOLDRULE_H
