#ifndef DEPENDENCYMEDIATOR_H
#define DEPENDENCYMEDIATOR_H

class DependencyMediator : WorkflowMediator {

private:
	vector<DependencyRules> rules;
	map<string, WorkItem*> items;

public:
	void registerItem(WorkItem* item);

	void itemChanged(WorkItem& item);

	void openChildren(SubProcess& process);

private:
	bool hasIncomingRule(string id);

	bool ruleSatisfied(const DependencyRule& rule);
};

#endif
