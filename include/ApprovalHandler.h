#ifndef APPROVALHANDLER_H
#define APPROVALHANDLER_H

/**
 * @file ApprovalHandler.h
 * @brief Abstract Handler interface for the Chain of Responsibility approval workflow.
 * @author Enock Nyamweya
 * @version 2.0
 */

#include <memory>

class ApprovalRequest;
class Participant;

/**
 * @class ApprovalHandler
 * @brief Base handler in the Chain of Responsibility for multi-level approvals.
 *
 * Ownership of the succeeding handler in the chain is managed via std::unique_ptr,
 * ensuring automatic recursive destruction with zero memory leaks (pure RAII).
 */
class ApprovalHandler {
private:
    std::unique_ptr<ApprovalHandler> nextHandler; ///< Owned successor in chain.

public:
    ApprovalHandler()
        : nextHandler(nullptr) {}

    virtual ~ApprovalHandler() = default;

    // Disallow copying to maintain strict linear chain ownership
    ApprovalHandler(const ApprovalHandler&) = delete;
    ApprovalHandler& operator=(const ApprovalHandler&) = delete;

    /**
     * @brief Chains the next handler taking unique ownership.
     * @param next Unique pointer to the next handler.
     */
    void setNext(std::unique_ptr<ApprovalHandler> next) {
        nextHandler = std::move(next);
    }

    /**
     * @brief Backward-compatible overload accepting raw pointer and taking ownership.
     * @param next Pointer to the next handler.
     */
    void setNext(ApprovalHandler* next) {
        nextHandler.reset(next);
    }

    /**
     * @brief Gets non-owning pointer to the next handler.
     * @return ApprovalHandler* Non-owning raw pointer.
     */
    ApprovalHandler* getNext() const {
        return nextHandler.get();
    }

    /**
     * @brief Handles or forwards the approval request.
     * @param request Approval context payload.
     * @return std::shared_ptr<Participant> Assigned approver, or nullptr.
     */
    virtual std::shared_ptr<Participant> handle(ApprovalRequest& request) = 0;
};

#endif // APPROVALHANDLER_H
