#include "WorkFlowInstance.h"
#include "createState.h"
#include "AvailableState.h"
#include "AssignedState.h"
#include "InProgresssState.h"
#include "EscalatedState.h"
#include "CompletedState.h"
#include "RejectedState.h"
#include"cancelledState.h"
#include "SubProcess.h"
#include<iostream>
using namespace std;

using namespace std;


WorkFlowInstance::WorkFlowInstance(const string& id,const WorkflowDefinition* definition,SubProcess* root,DependencyMediator* mediator,const InstanceData& data,ParticipantDirectory* directory)
{
    Id = id;
    this->definition = definition;
    rootProcess = root;
    this->mediator = mediator;
    this->data = data;
    particpants = directory;
}


WorkFlowInstance::~WorkFlowInstance()
{
    delete rootProcess;
    delete mediator;

    rootProcess = NULL;
    mediator = NULL;
}


bool WorkFlowInstance::start()
{
    if (rootProcess==NULL)
    {
        return false;
    }
    return rootProcess->makeAvailable();
}


const string& WorkFlowInstance::getId() const
{
    return Id;
}
InstanceData& WorkFlowInstance::getData()
{
    return data;
}
WorkItem* WorkFlowInstance::findItem(const string& itemId) const
{
    if (rootProcess == NULL)
    {
        return NULL;
    }

    return findItemRecursive(rootProcess, itemId);
}
//helper function
WorkItem* WorkFlowInstance::findItemRecursive(WorkItem* current,const string& itemId) const
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
string WorkFlowInstance::status() const
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
void WorkFlowInstance::captureItem(WorkItem* item,map<string, string>& states,map<string, string>& assignees,map<string, string>& outcomes) const
{
    if (item == NULL)
    {
        return;
    }
    states[item->getId()] =item->getName();

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


WorkflowSnapshot* WorkFlowInstance::createSnapshot(const string& label)
{
    map<string, string> itemStates;
    map<string, string> assignees;
    map<string, string> outcomes;

    captureItem(rootProcess,itemStates,assignees,outcomes);

    return new WorkflowSnapshot(itemStates,assignees,outcomes,data,label);
}

void WorkFlowInstance::restoreSnapshot(const WorkflowSnapshot& snapshot)
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
    for (map<string, string>::const_iterator it =snapshot.outcome.begin();it != snapshot.outcome.end();++it)
    {
        WorkItem* item =findItem(it->first);

        if (item != NULL)
        {
            item->outcome =it->second;
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

        if (particpants == NULL)
        {
            continue;
        }

        Participant* participant =particpants->findById(it->second);

        task->assignee =participant;
    }
}


void WorkFlowInstance::clearAssignees(WorkItem* item)
{
    if (item == NULL)
    {
        return;
    }

    Task* task =dynamic_cast<Task*>(item);

    if (task != NULL)
    {
        task->assignee = NULL;
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
WorkItemState* WorkFlowInstance::createStateFromName(const string& stateName) const
{
    if (stateName == "Created")
    {
        return new createState();
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
        return new InProgresssState();
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
        return new cancelledState();
    }

    return NULL;
}