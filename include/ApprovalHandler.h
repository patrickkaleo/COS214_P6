#ifndef APPROVALHANDLER_H
#define APPROVALHANDLER_H

/**
 * @file ApprovalHandler.h
 * @brief Abstract Handler interface for the Chain of Responsibility approval workflow.
 * @author Enock Nyamweya
 * @version 1.0
 */

class ApprovalRequest;
class Participant;

class ApprovalHandler {
private:
    ApprovalHandler* nextHandler;

public:
    ApprovalHandler()
        : nextHandler(nullptr) {}

    virtual ~ApprovalHandler() {
        delete nextHandler;
        nextHandler = nullptr;
    }

    void setNext(ApprovalHandler* next) {
        nextHandler = next;
    }

    ApprovalHandler* getNext() const {
        return nextHandler;
    }

    virtual Participant* handle(ApprovalRequest& request) = 0;
};

#endif // APPROVALHANDLER_H
