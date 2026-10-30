#ifndef WORKFLOWSNAPSHOT_H
#define WORKFLOWSNAPSHOT_H

#include <string>
#include <vector>
#include <map>
#include "InstanceData.h"

class WorkflowSnapshot
{

private:
	std::string label;
	std::vector<std::string> savedPayload;
	InstanceData data;
	std::map<std::string, std::string> outcome;
	std::map<std::string, std::string> assignees;
	std::map<std::string, std::string> itemStates;

public:
	std::string getLabel();
};

#endif
