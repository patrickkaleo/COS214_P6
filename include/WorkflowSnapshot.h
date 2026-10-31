#ifndef WORKFLOWSNAPSHOT_H
#define WORKFLOWSNAPSHOT_H

#include <map>
#include <string>
#include "InstanceData.h"

class WorkflowInstance;

class WorkflowSnapshot
{
	friend class WorkflowInstance;

private:
	std::string label;
	InstanceData data;
	std::map<std::string, std::string> outcomes;
	std::map<std::string, std::string> assignees;
	std::map<std::string, std::string> itemStates;

public:
	WorkflowSnapshot(const std::map<std::string, std::string> &itemStates,
									 const std::map<std::string, std::string> &assignees,
									 const std::map<std::string, std::string> &outcomes,
									 const InstanceData &data,
									 const std::string &label);
	std::string getLabel() const;
};
#endif
