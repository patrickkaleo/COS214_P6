#ifndef APPROVALROUTING_H
#define APPROVALROUTING_H

/**
 * @file ApprovalRouting.h
 * @brief Concrete strategy delegating task allocation to a Chain of Responsibility.
 * @author Enock Nyamweya
 * @version 2.0
 */

#include "RoutingStrategy.h"
#include <memory>

class ApprovalHandler;

/**
 * @class ApprovalRouting
 * @brief Integrates the Strategy pattern with the Chain of Responsibility pattern.
 *
 * For approval workflows (e.g. bursary allocation, expense approvals, medical authorizations),
 * the decision of who approves depends on escalation level, approval limits, or department.
 * ApprovalRouting encapsulates this delegation: it owns the head of an ApprovalHandler chain
 * via std::unique_ptr, constructs an ApprovalRequest, and asks the chain to handle and return
 * the proper approver.
 */
class ApprovalRouting : public RoutingStrategy {
private:
    std::unique_ptr<ApprovalHandler> chain; ///< Head of the ApprovalHandler chain (owned via unique_ptr).

public:
    /**
     * @brief Default constructor with null handler chain.
     */
    ApprovalRouting();

    /**
     * @brief Parameterized constructor taking exclusive ownership of an ApprovalHandler chain.
     * @param handlerChain Unique pointer to the first handler in the chain.
     */
    explicit ApprovalRouting(std::unique_ptr<ApprovalHandler> handlerChain);

    /**
     * @brief Backward-compatible constructor taking a raw pointer (assumes ownership).
     * @param handlerChain Pointer to the first handler in the chain.
     */
    explicit ApprovalRouting(ApprovalHandler* handlerChain);

    /**
     * @brief Destructor. Handled automatically via RAII (unique_ptr).
     */
    virtual ~ApprovalRouting() override = default;

    ApprovalRouting(const ApprovalRouting&) = delete;
    ApprovalRouting& operator=(const ApprovalRouting&) = delete;

    /**
     * @brief Routes the approval task by passing an ApprovalRequest through the handler chain.
     * @param task The task requiring approval.
     * @param data Instance runtime data containing request details.
     * @param directory The participant directory.
     * @return std::shared_ptr<Participant> Shared pointer to the chosen approver, or nullptr.
     */
    virtual Participant* select(const Task& task,
                                const InstanceData& data,
                                const ParticipantDirectory& directory) override;

    /**
     * @brief Clones this strategy instance as a newly allocated strategy.
     * @return RoutingStrategy* Cloned strategy.
     */
    virtual RoutingStrategy* clone() const override;

    /**
     * @brief Gets non-owning pointer to the head of the handler chain.
     * @return ApprovalHandler* Non-owning pointer.
     */
    ApprovalHandler* getChain() const;

    /**
     * @brief Sets or updates the handler chain head taking unique ownership.
     * @param handlerChain The new handler chain head.
     */
    void setChain(std::unique_ptr<ApprovalHandler> handlerChain);

    /**
     * @brief Backward-compatible overload taking raw pointer (assumes ownership).
     * @param handlerChain The new handler chain head.
     */
    void setChain(ApprovalHandler* handlerChain);
};

#endif // APPROVALROUTING_H
