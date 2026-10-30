/**
 * @file ApprovalRouting.cpp
 * @brief Implementation of approval-based task routing strategy delegating to Chain of Responsibility.
 * @author Enock Nyamweya
 * @version 2.0
 */

#include "ApprovalRouting.h"
#include "ApprovalHandler.h"
#include "ApprovalRequest.h"
#include "InstanceData.h"
#include "ParticipantDirectory.h"

ApprovalRouting::ApprovalRouting()
    : chain(nullptr) {
}

ApprovalRouting::ApprovalRouting(std::unique_ptr<ApprovalHandler> handlerChain)
    : chain(std::move(handlerChain)) {
}

ApprovalRouting::ApprovalRouting(ApprovalHandler* handlerChain)
    : chain(handlerChain) {
}

Participant* ApprovalRouting::select(const Task& task,
                                     const InstanceData& data,
                                     const ParticipantDirectory& directory) {
    (void)directory;

    if (!chain) {
        return nullptr;
    }

    ApprovalRequest request;
    request.item = &task;
    request.data = &data;
    request.level = 0;
    request.directory = &directory;

    auto approver = chain->handle(request);
    return approver ? approver.get() : nullptr;
}

RoutingStrategy* ApprovalRouting::clone() const {
    return new ApprovalRouting(nullptr);
}

ApprovalHandler* ApprovalRouting::getChain() const {
    return chain.get();
}

void ApprovalRouting::setChain(std::unique_ptr<ApprovalHandler> handlerChain) {
    chain = std::move(handlerChain);
}

void ApprovalRouting::setChain(ApprovalHandler* handlerChain) {
    chain.reset(handlerChain);
}
