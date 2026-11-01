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
    : chain(nullptr), selectedDeadlineHours(0), selectedLevel(-1) {
}

ApprovalRouting::ApprovalRouting(ApprovalHandler* handlerChain)
    : chain(handlerChain), selectedDeadlineHours(0), selectedLevel(-1) {
}

ApprovalRouting::~ApprovalRouting() {
    delete chain;
    chain = nullptr;
}

Participant* ApprovalRouting::select(const Task& task,
                                     const InstanceData& data,
                                     const ParticipantDirectory& directory) {
    if (!chain) {
        selectedDeadlineHours = 0;
        selectedLevel = -1;
        return nullptr;
    }

    // Encapsulate approval context in an ApprovalRequest value object.
    // Each escalation of the task moves the request one handler further along the chain.
    ApprovalRequest request(&task, &data, task.getEscalationLevel(), &directory);

    // Delegate processing to the Chain of Responsibility
    Participant* participant = chain->handle(request);
    selectedDeadlineHours = participant ? request.deadlineHours : 0;
    selectedLevel = participant ? request.selectedLevel : -1;
    return participant;
}

RoutingStrategy* ApprovalRouting::clone() const {
    // Each instance's task gets its own copy of the definition's chain.
    return new ApprovalRouting(chain ? chain->clone() : nullptr);
}

ApprovalHandler* ApprovalRouting::getChain() const {
    return chain;
}

int ApprovalRouting::getDeadlineHours() const {
    return selectedDeadlineHours;
}

int ApprovalRouting::getSelectedLevel() const {
    return selectedLevel;
}

void ApprovalRouting::setChain(ApprovalHandler* handlerChain) {
    if (chain != handlerChain) {
        delete chain;
        chain = handlerChain;
        selectedDeadlineHours = 0;
        selectedLevel = -1;
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
