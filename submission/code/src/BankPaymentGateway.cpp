/**
 * @file BankPaymentGateway.cpp
 * @brief Simulated bank EFT gateway used by BankGatewayAdapter.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "BankPaymentGateway.h"
#include <cctype>
#include <sstream>

BankPaymentGateway::BankPaymentGateway(long long limitCents)
    : transferLimitCents(limitCents), online(true), nextTransaction(1) {
}

EftResponse BankPaymentGateway::submitEft(const EftInstruction& instruction) {
    EftResponse response;
    response.accepted = false;

    if (!online) {
        response.errorCode = "GATEWAY_DOWN";
        return response;
    }

    // Account numbers are 9 to 11 digits
    const std::string& acc = instruction.beneficiaryAccount;
    bool validAccount = acc.size() >= 9 && acc.size() <= 11;
    for (std::string::size_type i = 0; validAccount && i < acc.size(); ++i) {
        validAccount = std::isdigit(static_cast<unsigned char>(acc[i])) != 0;
    }
    if (!validAccount || instruction.branchCode.empty()) {
        response.errorCode = "ACC_INVALID";
        return response;
    }

    if (instruction.amountCents <= 0) {
        response.errorCode = "AMOUNT_INVALID";
        return response;
    }
    if (instruction.amountCents > transferLimitCents) {
        response.errorCode = "LIMIT_EXCEEDED";
        return response;
    }

    std::ostringstream id;
    id << "EFT" << 100000 + nextTransaction++;
    response.accepted = true;
    response.transactionId = id.str();
    return response;
}

void BankPaymentGateway::setOnline(bool isOnline) {
    online = isOnline;
}
