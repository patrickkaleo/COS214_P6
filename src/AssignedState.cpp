/**
 * @file AssignedState.cpp
 * @brief Transitions available from the Assigned state.
 * @author Angela Ramaboea
 * @version 1.0
 */

#include "AssignedState.h"
#include "WorkItem.h"
#include "WorkflowEvent.h"
#include "CancelledState.h"
#include "EscalatedState.h"
#include "InProgressState.h"

bool AssignedState::assign(WorkItem &item, Participant *participant)
{
    if (participant == nullptr)
    {
        return false;
    }
    item.setAssignee(participant);
    item.changeState(new AssignedState(), EventType::ASSIGNED);
    return true;
}

bool AssignedState::start(WorkItem &item)
{
    item.changeState(new InProgressState(), EventType::STARTED);
    return true;
}

bool AssignedState::cancel(WorkItem &item)
{
    item.changeState(new CancelledState(), EventType::CANCELLED);
    return true;
}

bool AssignedState::escalate(WorkItem &item)
{
    item.incrementEscalation();
    item.setAssignee(nullptr);
    item.changeState(new EscalatedState(), EventType::ESCALATED);
    return true;
}

std::string AssignedState::name() const
{
    return "Assigned";
}

WorkItemState *AssignedState::clone() const
{
    return new AssignedState(*this);
}
