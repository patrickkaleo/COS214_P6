#include "HistoryLog.h"
using namespace std;

void HistoryLog::pushSnapshots(const string & InstanceID, WorkflowSnapshot* snapshot)
{
    if(snapshot==NULL)
    {
        return;
    }
    snapshots[InstanceID].push_back(snapshot);

}
WorkflowSnapshot* HistoryLog::popSnapshots(const string& instanceId)
{
    map<string,vector<WorkflowSnapshot*>>::iterator it;
    it= snapshots.find(instanceId);
    if(it==snapshots.end())
    {
        return NULL;
    }
    if(it->second.empty())
    {
        return NULL;
    }
    WorkflowSnapshot* snapshotsl=it->second.back();
    it->second.pop_back();
    return snapshotsl;

}
const WorkflowSnapshot* HistoryLog::latest(const string & InstanceID) const
{
    map<string,vector<WorkflowSnapshot*>>::const_iterator it;
    it= snapshots.find(InstanceID);
    if(it==snapshots.end())
    {
        return NULL;
    }
    if(it->second.empty())
    {
        return NULL;
    }
    return it->second.back();
}
HistoryLog::~HistoryLog()
{
    for(map<string,vector<WorkflowSnapshot*>>::iterator it=snapshots.begin();it!=snapshots.end();++it)
    {
        for(size_t i=0;i<it->second.size();i++)
        {
            delete it->second[i];
        }
    }
}
