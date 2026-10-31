/**
 * @file PriorityDecorator.cpp
 * @brief Concrete Decorator that adds a priority level to any WorkItem.
 * @details When the item becomes available it is flagged as urgent before the
 *          call is forwarded to the inner item.
 * @version 1.0
 */

#include "WorkItemDecorator.h"
#include "PriorityDecorator.h"

#include "Participant.h"
#include "WorkflowEvent.h"

#include <iostream>
#include <string>

using namespace std;

PriorityDecorator::PriorityDecorator(WorkItem* inner, int level)
    : WorkItemDecorator(inner),
      level_(level)
{}

int PriorityDecorator::getPriority() const {
    return level_;
}

void PriorityDecorator::onAvailable() {
    // Extra behaviour
    cout << "[PriorityDecorator] \"" << getName() << "\" flagged as priority level " << level_ << "\n";
    // Then forward
    WorkItemDecorator::onAvailable();
}

WorkItem* PriorityDecorator::clone() const {
    WorkItem* innerClone = inner_ ? inner_->clone() : nullptr;
    return new PriorityDecorator(innerClone, level_);
}