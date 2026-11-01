#ifndef BANKPAYMENTGATEWAY_H
#define BANKPAYMENTGATEWAY_H

/**
 * @file BankPaymentGateway.h
 * @brief Adaptee: simulated bank EFT gateway with its own instruction/response types.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include <string>

/// EFT instruction in the bank's own format (amounts in cents).
struct EftInstruction {
    std::string beneficiaryAccount;
    std::string branchCode;
    long long amountCents;
    std::string reference;
};

/// Bank's reply to an EFT instruction.
struct EftResponse {
    bool accepted;
    std::string transactionId; ///< Set when accepted.
    std::string errorCode;     ///< "ACC_INVALID", "LIMIT_EXCEEDED", "AMOUNT_INVALID" or "GATEWAY_DOWN".
};

/**
 * @class BankPaymentGateway
 * @brief Test stub standing in for a bank's electronic funds transfer gateway.
 *
 * Works in cents with its own request and response structs. BankGatewayAdapter
 * translates the engine's string-based requests into this API.
 */
class BankPaymentGateway {
public:
    /// @param limitCents Largest single transfer accepted, in cents.
    explicit BankPaymentGateway(long long limitCents = 10000000LL);

    /**
     * @brief Submits a transfer.
     * @param instruction Transfer details.
     * @return EftResponse Acceptance with a transaction id, or an error code.
     */
    EftResponse submitEft(const EftInstruction& instruction);

    /// Simulates the gateway going offline (false) or coming back (true).
    void setOnline(bool isOnline);

private:
    long long transferLimitCents;
    bool online;
    int nextTransaction;
};

#endif // BANKPAYMENTGATEWAY_H
