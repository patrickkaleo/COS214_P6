/**
 * @file ApprovalChainIterator.cpp
 * @brief Linear traversal of an approval chain.
 * @author Angelo Anthony
 * @version 1.0
 */

#include "ApprovalChainIterator.h"
#include "ApprovalHandler.h"

ApprovalChainIterator::ApprovalChainIterator(ApprovalHandler* chainHead)
    : head(chainHead),
      position(chainHead) {
}

void ApprovalChainIterator::first() {
    position = head;
}

void ApprovalChainIterator::next() {
    if (position != nullptr) {
        position = position->getNext();
    }
}

bool ApprovalChainIterator::isDone() const {
    return position == nullptr;
}

ApprovalHandler* ApprovalChainIterator::current() const {
    return position;
}
