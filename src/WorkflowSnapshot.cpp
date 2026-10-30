#include "WorkflowSnapshot.h"
using namespace std;
WorkflowSnapshot::WorkflowSnapshot(const std::map<std::string, std::string>& itemStates,const std::map<std::string, std::string>& assignees,const std::map<std::string, std::string>& outcomes,const InstanceData& data,const std::string& label)
    : itemStates(itemStates),
      assignees(assignees),
      outcomes(outcomes),
      data(data),
      label(label)
{
}
string WorkflowSnapshot::getLabel() const
{
    return label;
}