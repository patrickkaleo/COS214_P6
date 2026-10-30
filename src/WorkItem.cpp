/**
 * @file WorkItem.cpp
 * @brief WorkItem lifecycle delegation to WorkItemState and event publication.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "WorkItem.h"
#include "WorkItemState.h"
#include "CreatedState.h"
#include "EventDispatcher.h"

WorkItem::WorkItem(
    const std::string &id,
    const std::string &itemName)
    : instanceId_(""),
      assignee_(nullptr),
      escalationLevel_(0),
      id_(id),
      name_(itemName),
      outcome_(""),
      state_(new CreatedState()),
      parent_(nullptr),
      mediator_(nullptr)
{
}

WorkItem::WorkItem(const WorkItem &other)
    : instanceId_(other.instanceId_),
      assignee_(other.assignee_),
      escalationLevel_(other.escalationLevel_),
      id_(other.id_),
      name_(other.name_),
      outcome_(other.outcome_),
      state_(other.state_
                 ? other.state_->clone()
                 : nullptr),
      parent_(nullptr),
      mediator_(nullptr)
{
}

WorkItem::~WorkItem()
{
    delete state_;
}
bool WorkItem::makeAvailable()
{
    if (!state_->makeAvailable(*this))
    {
        return false;
    }
    onAvailable();
    return true;
}

bool WorkItem::assign(Participant *participant)
{
    return state_->assign(*this, participant);
}

bool WorkItem::start()
{
    return state_->start(*this);
}

bool WorkItem::complete(const std::string &result)
{
    return state_->complete(*this, result);
}

bool WorkItem::reject(const std::string &reason)
{
    return state_->reject(*this, reason);
}

bool WorkItem::cancel()
{
    return state_->cancel(*this);
}

bool WorkItem::escalate()
{
    return state_->escalate(*this);
}

void WorkItem::setState(WorkItemState *next)
{
    if (next == state_)
        return;

    delete state_;
    state_ = next;
}

void WorkItem::changeState(
    WorkItemState *next,
    EventType event)
{
    setState(next);

    EventDispatcher::getInstance().notifyListeners(
        WorkflowEvent(
            event,
            instanceId_,
            id_,
            name_,
            outcome_));
}

void WorkItem::setAssignee(Participant *participant)
{
    assignee_ = participant;
}

// void WorkItem::setOutcome(const std::string &result)
// {
//     outcome_ = result;
// }

void WorkItem::incrementEscalation()
{
    ++escalationLevel_;
}

bool WorkItem::isFinished() const
{
    return state_->isFinal();
}

std::string WorkItem::getStateName() const
{
    return state_->name();
}

// std::string WorkItem::getName() const
// {
//     return name_;
// }

// std::string WorkItem::getOutcome() const
// {
//     return outcome_;
// }

// Participant *WorkItem::getAssignee() const
// {
//     return assignee_;
// }

// int WorkItem::getEscalationLevel() const
// {
//     return escalationLevel_;
// }

// std::string WorkItem::getInstanceId() const
// {
//     return instanceId_;
// }

// void WorkItem::setInstanceId(const std::string &id)
// {
//     instanceId_ = id;
// }

// SubProcess *WorkItem::getParent() const
// {
//     return parent_;
// }

// void WorkItem::setParent(SubProcess *p)
// {
//     parent_ = p;
// }
