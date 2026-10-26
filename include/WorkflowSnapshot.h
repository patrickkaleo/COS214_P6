#ifndef WORKFLOWSNAPSHOT_H
#define WORKFLOWSNAPSHOT_H
#include "WorkFlowInstance.h"
class WorkflowSnapshot 
{

private:
	friend class WorkFlowInstance;
	string label;
	vector<string> savedPayload;
	InstanceData data;
	map<string, string> outcome;
	map<string, string> assignees;
	map<string, string> itemStates;
	WorkflowSnapshot(const std::map<std::string, std::string>& itemStates,const std::map<std::string, std::string>& assignees,const std::map<std::string, std::string>& outcomes,const InstanceData& data,const std::string& label);
public:
	string getLabel()const;

};

#endif
