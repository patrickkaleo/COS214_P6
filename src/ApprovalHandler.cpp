/**
 * @file ApprovalHandler.cpp
 * @brief Chain linking, forwarding, copying and iteration shared by all approval handlers.
 * @author Enock Nyamweya, Angelo Anthony
 * @version 1.1
 */

#include "ApprovalHandler.h"
#include "ApprovalChainIterator.h"

ApprovalHandler::ApprovalHandler()
    : nextHandler(nullptr) {
}

ApprovalHandler::~ApprovalHandler() {
    delete nextHandler;
}

ApprovalHandler* ApprovalHandler::setNext(ApprovalHandler* next) {
    if (next != nextHandler) {
        delete nextHandler;
        nextHandler = next;
    }
    return next;
}

ApprovalHandler* ApprovalHandler::getNext() const {
    return nextHandler;
}

std::string ApprovalHandler::describe() const {
    return "ApprovalHandler";
}

std::unique_ptr<Iterator<ApprovalHandler>> ApprovalHandler::createIterator() {
    return std::unique_ptr<Iterator<ApprovalHandler>>(new ApprovalChainIterator(this));
}

Participant* ApprovalHandler::passToNext(ApprovalRequest& request) {
    return nextHandler != nullptr ? nextHandler->handle(request) : nullptr;
}

ApprovalHandler* ApprovalHandler::withClonedNext(ApprovalHandler* copy) const {
    if (nextHandler != nullptr) {
        copy->setNext(nextHandler->clone());
    }
    return copy;
}
