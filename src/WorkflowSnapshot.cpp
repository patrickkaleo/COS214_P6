#include "WorkflowSnapshot.h"
using namespace std;

WorkflowSnapshot::WorkflowSnapshot(
    const std::map<std::string, std::string> &itemStates,
    const std::map<std::string, std::string> &assignees,
    const std::map<std::string, std::string> &outcomes,
    const InstanceData &data,
    const std::string &label)
    : label(label),
      data(data),
      outcomes(outcomes),
      assignees(assignees),
      itemStates(itemStates)
{
}
string WorkflowSnapshot::getLabel() const
{
    return label;
}