/**
 * @file ApprovalRouting.cpp
 * @brief Implementation of approval-based task routing strategy delegating to Chain of Responsibility.
 * @author Enock Nyamweya
 * @version 1.0
 */

#include "ApprovalRouting.h"
#include "ApprovalHandler.h"
#include "ApprovalRequest.h"
#include "InstanceData.h"
#include "ParticipantDirectory.h"

ApprovalRouting::ApprovalRouting()
    : chain(nullptr) {
}

ApprovalRouting::ApprovalRouting(ApprovalHandler* handlerChain)
    : chain(handlerChain) {
}

ApprovalRouting::~ApprovalRouting() {
    delete chain;
    chain = nullptr;
}

Participant* ApprovalRouting::select(const Task& task,
                                     const InstanceData& data,
                                     const ParticipantDirectory& directory) {
    (void)directory; // Passed via ApprovalRequest context

    if (!chain) {
        return nullptr;
    }

    // Encapsulate approval context in an ApprovalRequest value object
    ApprovalRequest request;
    request.item = &task;
    request.data = &data;
    request.level = 0;

    // Delegate processing to the Chain of Responsibility
    return chain->handle(request);
}

RoutingStrategy* ApprovalRouting::clone() const {
    // Handler chains are composed dynamically per workflow definition.
    // Return a new ApprovalRouting container.
    return new ApprovalRouting(nullptr);
}

ApprovalHandler* ApprovalRouting::getChain() const {
    return chain;
}

void ApprovalRouting::setChain(ApprovalHandler* handlerChain) {
    if (chain != handlerChain) {
        delete chain;
        chain = handlerChain;
    }
}
