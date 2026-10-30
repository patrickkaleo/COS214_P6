#ifndef APPROVALHANDLER_H
#define APPROVALHANDLER_H

/**
 * @file ApprovalHandler.h
 * @brief Abstract Handler interface for the Chain of Responsibility approval workflow.
 * @author Enock Nyamweya, Angelo Anthony
 * @version 1.1
 */

#include <memory>
#include <string>
#include "Iterator.h"

class ApprovalRequest;
class Participant;

/**
 * @class ApprovalHandler
 * @brief Handler of the GoF Chain of Responsibility (Behavioural) pattern.
 *
 * Approval may need one person or several levels of authority. The task asking for
 * approval only knows the first handler. Each handler either finds an approver or
 * passes the request on, so the number, order and rules of the levels are pure
 * configuration.
 *
 * A handler owns the rest of the chain after it, and deleting the head deletes
 * the whole chain.
 *
 * @see ConcreteApprovalHandler, ApprovalRouting, ApprovalChainIterator
 */
class ApprovalHandler {
private:
    ApprovalHandler* nextHandler; ///< Next level of authority (owned, may be null).

public:
    ApprovalHandler();
    virtual ~ApprovalHandler();

    ApprovalHandler(const ApprovalHandler&) = delete;
    ApprovalHandler& operator=(const ApprovalHandler&) = delete;

    /**
     * @brief Sets the next handler, deleting any handler it replaces.
     *
     * Returns its argument so a chain reads in order:
     * @code
     *   officer->setNext(manager)->setNext(committee);
     * @endcode
     *
     * @param next Next handler (ownership is taken), or nullptr to end the chain.
     * @return ApprovalHandler* The handler passed in.
     */
    ApprovalHandler* setNext(ApprovalHandler* next);

    ApprovalHandler* getNext() const;

    /**
     * @brief Finds an approver for the request, or passes it along the chain.
     * @param request The approval being asked for.
     * @return Participant* The approver, or nullptr if no handler in the chain can approve.
     */
    virtual Participant* handle(ApprovalRequest& request) = 0;

    /**
     * @brief Copies this handler and every handler after it.
     *
     * Each workflow instance gets its own chain when the definition's tasks are cloned.
     *
     * @return ApprovalHandler* Newly allocated copy of the chain from this handler on.
     */
    virtual ApprovalHandler* clone() const = 0;

    /// @return One-line description of this level, shown when listing a chain.
    virtual std::string describe() const;

    /**
     * @brief Creates an iterator over this handler and every handler after it.
     * @return Iterator whose first element is this handler.
     */
    std::unique_ptr<Iterator<ApprovalHandler>> createIterator();

protected:
    /// @return The next handler's answer, or nullptr at the end of the chain.
    Participant* passToNext(ApprovalRequest& request);

    /**
     * @brief Helper for clone(): attaches a copy of the rest of this chain to @p copy.
     * @param copy Freshly made copy of this handler.
     * @return ApprovalHandler* @p copy.
     */
    ApprovalHandler* withClonedNext(ApprovalHandler* copy) const;
};

#endif // APPROVALHANDLER_H
