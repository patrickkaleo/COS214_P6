#ifndef DEPENDENCYMEDIATOR_H
#define DEPENDENCYMEDIATOR_H
#include "WorkflowMediator.h"
#include "DependencyRule.h"

#include <vector>
#include <map>

class DependencyMediator : public WorkflowMediator
{

private:
	std::vector<DependencyRule> rules;
	std::map<std::string, WorkItem *> items;

public:
	void registerItem(WorkItem *item);

	void itemChanged(WorkItem &item);

	void openChildren(SubProcess &process);

private:
	bool hasIncomingRule(std::string id);

	bool ruleSatisfied(const DependencyRule &rule);
};

#endif
