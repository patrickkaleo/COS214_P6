/**
 * @file BankGatewayAdapter.cpp
 * @brief Translates payment requests into BankPaymentGateway EFT instructions.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "BankGatewayAdapter.h"
#include <cmath>
#include <cstdlib>

namespace {

/// Parses a rand amount such as "1500" or "1500.75" into cents; returns false if malformed.
bool randsToCents(const std::string& amount, long long& cents) {
    if (amount.empty()) {
        return false;
    }
    char* end = nullptr;
    double rands = std::strtod(amount.c_str(), &end);
    if (end == amount.c_str() || *end != '\0' || !std::isfinite(rands)) {
        return false;
    }
    cents = std::llround(rands * 100.0);
    return true;
}

std::string describeBankError(const std::string& code) {
    if (code == "ACC_INVALID")    return "Beneficiary account details are invalid";
    if (code == "LIMIT_EXCEEDED") return "Amount exceeds the gateway transfer limit";
    if (code == "AMOUNT_INVALID") return "Payment amount must be positive";
    if (code == "GATEWAY_DOWN")   return "Bank gateway unavailable";
    return "Payment rejected by bank (" + code + ")";
}

} // namespace

BankGatewayAdapter::BankGatewayAdapter(BankPaymentGateway* adaptee)
    : gateway(adaptee != nullptr ? adaptee : new BankPaymentGateway()) {
}

ServiceResult BankGatewayAdapter::invoke(const ServiceRequest& request) {
    EftInstruction instruction;
    instruction.beneficiaryAccount = request.get("accountNumber");
    instruction.branchCode = request.get("branchCode");
    instruction.reference = request.get("reference");

    if (!randsToCents(request.get("amount"), instruction.amountCents)) {
        return ServiceResult::failure("Payment needs a numeric 'amount' in rands");
    }

    EftResponse response = gateway->submitEft(instruction);
    if (!response.accepted) {
        return ServiceResult::failure(describeBankError(response.errorCode));
    }

    std::map<std::string, std::string> out;
    out["paymentReference"] = response.transactionId;
    out["paymentStatus"] = "PAID";
    return ServiceResult::ok("Payment " + response.transactionId + " accepted", out);
}

std::string BankGatewayAdapter::name() const {
    return "PaymentGateway";
}

BankPaymentGateway& BankGatewayAdapter::getGateway() {
    return *gateway;
}
