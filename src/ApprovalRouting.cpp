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
#include "Task.h"

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
    if (!chain) {
        return nullptr;
    }

    // Encapsulate approval context in an ApprovalRequest value object.
    // Each escalation of the task moves the request one handler further along the chain.
    ApprovalRequest request(&task, &data, task.getEscalationLevel(), &directory);

    // Delegate processing to the Chain of Responsibility
    return chain->handle(request);
}

RoutingStrategy* ApprovalRouting::clone() const {
    // Each instance's task gets its own copy of the definition's chain.
    return new ApprovalRouting(chain ? chain->clone() : nullptr);
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

std::vector<std::string> ApprovalRouting::describeChain() const {
    std::vector<std::string> levels;
    if (!chain) {
        return levels;
    }
    std::unique_ptr<Iterator<ApprovalHandler>> it = chain->createIterator();
    for (it->first(); !it->isDone(); it->next()) {
        levels.push_back(it->current()->describe());
    }
    return levels;
}
