#ifndef APPROVALROUTING_H
#define APPROVALROUTING_H

/**
 * @file ApprovalRouting.h
 * @brief Concrete strategy delegating task allocation to a Chain of Responsibility.
 * @author Enock Nyamweya
 * @version 1.0
 */

#include "RoutingStrategy.h"

// Forward declaration of Chain of Responsibility handler
class ApprovalHandler;

/**
 * @class ApprovalRouting
 * @brief Integrates the Strategy pattern with the Chain of Responsibility pattern.
 *
 * For approval workflows (e.g. bursary allocation, expense approvals, medical authorizations),
 * the decision of who approves depends on escalation level, approval limits, or department.
 * ApprovalRouting encapsulates this delegation: it owns the head of an ApprovalHandler chain,
 * constructs an ApprovalRequest, and asks the chain to handle and return the proper approver.
 */
class ApprovalRouting : public RoutingStrategy {
private:
    ApprovalHandler* chain; ///< Head of the ApprovalHandler chain of responsibility (owned).

public:
    /**
     * @brief Default constructor with null handler chain.
     */
    ApprovalRouting();

    /**
     * @brief Parameterized constructor taking ownership of an ApprovalHandler chain.
     * @param handlerChain Pointer to the first handler in the chain of responsibility.
     */
    explicit ApprovalRouting(ApprovalHandler* handlerChain);

    /**
     * @brief Destructor. Safely cleans up the owned ApprovalHandler chain.
     */
    virtual ~ApprovalRouting() override;

    /**
     * @brief Routes the approval task by passing an ApprovalRequest through the handler chain.
     *
     * Constructs an ApprovalRequest using the task's escalation level and instance data,
     * then executes @c chain->handle(request).
     *
     * @param task The task requiring approval.
     * @param data Instance runtime data containing request details (e.g. amount, severity).
     * @param directory The participant directory.
     * @return Participant* Pointer to the chosen approver participant, or nullptr if unhandled.
     */
    virtual Participant* select(const Task& task,
                                const InstanceData& data,
                                const ParticipantDirectory& directory) override;

    /**
     * @brief Clones this strategy instance.
     * @return RoutingStrategy* Newly allocated copy of this strategy.
     */
    virtual RoutingStrategy* clone() const override;

    /**
     * @brief Gets the current head of the handler chain.
     * @return ApprovalHandler* Pointer to the handler.
     */
    ApprovalHandler* getChain() const;

    /**
     * @brief Sets or updates the handler chain head (takes ownership).
     * @param handlerChain The new handler chain head.
     */
    void setChain(ApprovalHandler* handlerChain);
};

#endif // APPROVALROUTING_H
