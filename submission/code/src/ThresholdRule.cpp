/**
 * @file ThresholdRule.cpp
 * @brief Numeric limit check against instance data.
 * @author Angelo Anthony
 * @version 1.0
 */

#include "ThresholdRule.h"
#include <cstdlib>
#include <sstream>
#include "ApprovalRequest.h"
#include "InstanceData.h"

ThresholdRule::ThresholdRule(const std::string& fieldName, double limit)
    : field(fieldName),
      maxValue(limit) {
}

bool ThresholdRule::accepts(const ApprovalRequest& request) const {
    if (request.data == nullptr || !request.data->has(field)) {
        return false;
    }
    const std::string text = request.data->get(field);
    char* end = nullptr;
    double value = std::strtod(text.c_str(), &end);
    if (text.empty() || *end != '\0') {
        return false; // not a number: let a higher level look at it
    }
    return value <= maxValue;
}

ApprovalRule* ThresholdRule::clone() const {
    return new ThresholdRule(*this);
}

std::string ThresholdRule::describe() const {
    std::ostringstream out;
    out << field << " <= " << maxValue;
    return out.str();
}

std::string ThresholdRule::getField() const {
    return field;
}

double ThresholdRule::getMaxValue() const {
    return maxValue;
}
