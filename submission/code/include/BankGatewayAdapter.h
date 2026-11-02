#ifndef BANKGATEWAYADAPTER_H
#define BANKGATEWAYADAPTER_H

/**
 * @file BankGatewayAdapter.h
 * @brief Adapter from ExternalService to the bank EFT gateway (bursary payments).
 * @author Angela Ramaboea
 * @version 1.0
 */

#include <memory>
#include "ExternalService.h"
#include "BankPaymentGateway.h"

/**
 * @class BankGatewayAdapter
 * @brief Pays approved amounts through the bank's EFT gateway.
 *
 * Adapter role in the GoF Adapter pattern: wraps BankPaymentGateway (Adaptee).
 * It converts the rand amount string in the request into cents, builds an
 * EftInstruction and maps the bank's error codes to readable messages.
 *
 * Request parameters: @c accountNumber, @c branchCode, @c amount (rands, e.g. "45000.50"),
 * optional @c reference.
 * Outputs on success: @c paymentReference, @c paymentStatus = "PAID".
 *
 * Any bank rejection is a failed result, which the engine treats as the payment
 * exception path (restore snapshot and flag for follow-up).
 *
 * Registered as "PaymentGateway".
 */
class BankGatewayAdapter : public ExternalService {
private:
    std::unique_ptr<BankPaymentGateway> gateway;

public:
    /// @param adaptee Gateway to wrap (ownership taken); a default gateway is created if nullptr.
    explicit BankGatewayAdapter(BankPaymentGateway* adaptee = nullptr);

    ServiceResult invoke(const ServiceRequest& request) override;
    std::string name() const override;

    /// Access to the wrapped gateway, e.g. to simulate an outage.
    BankPaymentGateway& getGateway();
};

#endif // BANKGATEWAYADAPTER_H
