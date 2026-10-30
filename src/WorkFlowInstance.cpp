#include "WorkFlowInstance.h"
#include "CreatedState.h"
#include "AvailableState.h"
#include "AssignedState.h"
#include "InProgressState.h"
#include "EscalatedState.h"
#include "CompletedState.h"
#include "RejectedState.h"
#include"CancelledState.h"
#include "SubProcess.h"
#include "Task.h"
#include "Participant.h"
#include<iostream>
using namespace std;

using namespace std;


WorkflowInstance::WorkflowInstance(const string& id,const WorkflowDefinition* definition,SubProcess* root,DependencyMediator* mediator,const InstanceData& data,ParticipantDirectory* directory)
{
    Id = id;
    this->definition = definition;
    rootProcess = root;
    this->mediator = mediator;
    this->data = data;
    participants = directory;
}


WorkflowInstance::~WorkflowInstance()
{
    delete rootProcess;
    delete mediator;

    rootProcess = NULL;
    mediator = NULL;
}


bool WorkflowInstance::start()
{
    if (rootProcess==NULL)
    {
        return false;
    }
    return rootProcess->makeAvailable();
}


const string& WorkflowInstance::getId() const
{
    return Id;
}
InstanceData& WorkflowInstance::getData()
{
    return data;
}
WorkItem* WorkflowInstance::findItem(const string& itemId) const
{
    if (rootProcess == NULL)
    {
        return NULL;
    }

    return findItemRecursive(rootProcess, itemId);
}
//helper function
WorkItem* WorkflowInstance::findItemRecursive(WorkItem* current,const string& itemId) const
{
    if (current == NULL)
    {
        return NULL;
    }
    if (current->getId() == itemId)
    {
        return current;
    }
    SubProcess* process =dynamic_cast<SubProcess*>(current);
    if (process == NULL)
    {
        return NULL;
    }

    for (int i = 0; i < process->childCount(); ++i)
    {
        WorkItem* found =findItemRecursive(process->getChild(i), itemId);
        if (found != NULL)
        {
            return found;
        }
    }
    return NULL;
}
string WorkflowInstance::status() const
{
    if (rootProcess == NULL)
    {
        return "invalid";
    }
    string stateName=rootProcess->getName();
    if (stateName=="Completed")
    {
        return "completed";
    }
    if (stateName == "Rejected" ||stateName == "Cancelled")
    {
        return "terminated";
    }
    return "running";
}
//helper
void WorkflowInstance::captureItem(WorkItem* item,map<string, string>& states,map<string, string>& assignees,map<string, string>& outcomes) const
{
    if (item == NULL)
    {
        return;
    }
    states[item->getId()] = item->getStateName();

    outcomes[item->getId()] =item->getOutcome();
    Task* task =dynamic_cast<Task*>(item);
    
    if (task != NULL &&task->getAssignee() != NULL)
    {
        assignees[item->getId()]=task->getAssignee()->getId();
    }

    // If this is a SubProcess,
    // recursively capture all children
    SubProcess* process =dynamic_cast<SubProcess*>(item);

    if (process != NULL)
    {
        for (int i = 0;i < process->childCount();++i)
        {
            captureItem(process->getChild(i), states,assignees, outcomes);
        }
    }
}


WorkflowSnapshot* WorkflowInstance::createSnapshot(const string& label)
{
    map<string, string> itemStates;
    map<string, string> assignees;
    map<string, string> outcomes;

    captureItem(rootProcess,itemStates,assignees,outcomes);

    return new WorkflowSnapshot(itemStates,assignees,outcomes,data,label);
}

void WorkflowInstance::restoreSnapshot(const WorkflowSnapshot& snapshot)
{
    // Restore InstanceData
    data = snapshot.data;

    // Restore WorkItem states
    for (map<string, string>::const_iterator it =snapshot.itemStates.begin();it != snapshot.itemStates.end();++it)
    {
        WorkItem* item =findItem(it->first);
        if (item == NULL)
        {
            continue;
        }

        WorkItemState* restoredState=createStateFromName(it->second);
        if (restoredState != NULL)
        {
            item->setState(restoredState);
        }
    }

    // Restore WorkItem outcomes
    for (map<string, string>::const_iterator it =snapshot.outcomes.begin();it != snapshot.outcomes.end();++it)
    {
        WorkItem* item =findItem(it->first);

        if (item != NULL)
        {
           item->setOutcome(it->second);
        }
    }

    // Clear assignments created after snapshot
    clearAssignees(rootProcess);

    // Restore saved assignments
    for (map<string, string>::const_iterator it =snapshot.assignees.begin();it != snapshot.assignees.end();++it)
    {
        WorkItem* item =findItem(it->first);

        Task* task = dynamic_cast<Task*>(item);

        if (task == NULL)
        {
            continue;
        }

        if (participants == NULL)
        {
            continue;
        }

        std::shared_ptr<Participant> participant =participants->findById(it->second);
        task->setAssignee(participant.get());
    }
}
void WorkflowInstance::clearAssignees(WorkItem* item)
{
    if (item == NULL)
    {
        return;
    }

    Task* task =dynamic_cast<Task*>(item);

    if (task != NULL)
    {
        task->setAssignee(nullptr);
    }

    SubProcess* process =dynamic_cast<SubProcess*>(item);
    if (process != NULL)
    {
        for (int i = 0;i < process->childCount();++i)
        {
            clearAssignees(process->getChild(i));
        }
    }
}
WorkItemState* WorkflowInstance::createStateFromName(const string& stateName) const
{
    if (stateName == "Created")
    {
        return new CreatedState();
    }

    if (stateName == "Available")
    {
        return new AvailableState();
    }

    if (stateName == "Assigned")
    {
        return new AssignedState();
    }

    if (stateName == "InProgress")
    {
        return new InProgressState;
    }

    if (stateName == "Escalated")
    {
        return new EscalatedState();
    }

    if (stateName == "Completed")
    {
        return new CompletedState();
    }

    if (stateName == "Rejected")
    {
        return new RejectedState();
    }

    if (stateName == "Cancelled")
    {
        return new CancelledState();
    }

    return NULL;
}