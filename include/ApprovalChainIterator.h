#ifndef APPROVALCHAINITERATOR_H
#define APPROVALCHAINITERATOR_H

/**
 * @file ApprovalChainIterator.h
 * @brief Iterator over the levels of an approval chain.
 * @author Angelo Anthony
 * @version 1.0
 */

#include "Iterator.h"

class ApprovalHandler;

/**
 * @class ApprovalChainIterator
 * @brief ConcreteIterator walking an ApprovalHandler chain from a given handler to its end.
 *
 * Lets clients list or count approval levels without following next pointers
 * themselves. Returned by ApprovalHandler::createIterator(). Does not own the chain.
 */
class ApprovalChainIterator : public Iterator<ApprovalHandler> {
private:
    ApprovalHandler* head;     ///< First handler visited (not owned).
    ApprovalHandler* position; ///< Current handler, nullptr when done.

public:
    /// @param head First handler; nullptr gives an empty traversal.
    explicit ApprovalChainIterator(ApprovalHandler* head);

    void first() override;
    void next() override;
    bool isDone() const override;
    ApprovalHandler* current() const override;
};

#endif // APPROVALCHAINITERATOR_H
