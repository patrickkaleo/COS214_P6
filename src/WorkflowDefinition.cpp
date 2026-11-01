#include "WorkflowDefinition.h"
#include "AuditDecorator.h"
#include "AuditLogger.h"
#include "DataRuleRouting.h"
#include "PriorityDecorator.h"
#include "RoleBasedRouting.h"
#include "SubProcess.h"
#include "Task.h"
#include "WorkItemDecorator.h"

#include <iostream>
#include <set>

namespace
{
void describeItem(WorkItem *item, int depth, set<WorkItem *> &seen)
{
	if (item == nullptr || !seen.insert(item).second)
	{
		return;
	}

	WorkItem *core = item;
	string wraps;
	while (WorkItemDecorator *layer = dynamic_cast<WorkItemDecorator *>(core))
	{
		seen.insert(core);
		if (PriorityDecorator *priority = dynamic_cast<PriorityDecorator *>(layer))
		{
			wraps += " [priority " + to_string(priority->getPriority()) + "]";
		}
		else if (dynamic_cast<AuditDecorator *>(layer) != nullptr)
		{
			wraps += " [audit]";
		}
		else
		{
			wraps += " [decorator]";
		}
		WorkItem *inner = layer->getInner();
		if (inner == nullptr || inner == core || seen.count(inner) != 0)
		{
			core = inner;
			break;
		}
		core = inner;
	}
	if (core != nullptr)
	{
		seen.insert(core);
	}

	for (int i = 0; i < depth; ++i)
	{
		cout << "  ";
	}
	cout << item->getId() << " | " << item->getName();
	SubProcess *process = dynamic_cast<SubProcess *>(core);
	Task *task = dynamic_cast<Task *>(core);
	if (process != nullptr)
	{
		cout << " [process]";
	}
	else if (task != nullptr)
	{
		cout << " [task]";
	}
	cout << wraps;
	if (task != nullptr)
	{
		if (task->isCheckpoint())
		{
			cout << " checkpoint";
		}
		if (!task->getServiceName().empty())
		{
			cout << " service=\"" << task->getServiceName() << "\"";
		}
		if (task->getDeadlineHours() > 0)
		{
			cout << " deadline=" << task->getDeadlineHours() << "h";
		}
		cout << " escalation=" << task->getEscalationLevel();
		RoutingStrategy *routing = task->getRouting();
		RoleBasedRouting *role = dynamic_cast<RoleBasedRouting *>(routing);
		DataRuleRouting *data = dynamic_cast<DataRuleRouting *>(routing);
		if (role != nullptr)
		{
			cout << " routing=role:" << role->getRole();
		}
		else if (data != nullptr)
		{
			cout << " routing=data:" << data->getField()
				 << " fallback=" << data->getFallbackRole();
			map<string, string> mapping = data->getMapping();
			for (map<string, string>::const_iterator rule = mapping.begin(); rule != mapping.end(); ++rule)
			{
				cout << " " << rule->first << "->" << rule->second;
			}
		}
		else if (routing != nullptr)
		{
			cout << " routing=set";
		}
	}
	cout << "\n";

	WorkItem *container = process != nullptr ? static_cast<WorkItem *>(process) : item;
	if (container == nullptr)
	{
		return;
	}
	for (int i = 0; i < container->childCount(); ++i)
	{
		WorkItem *child = container->getChild(i);
		if (child != nullptr && child != item && child != container)
		{
			describeItem(child, depth + 1, seen);
		}
	}
}

set<string> usedDefinitionIds;

bool reserveId(const string &id)
{
	if (id.empty() || usedDefinitionIds.count(id) != 0)
	{
		return false;
	}
	usedDefinitionIds.insert(id);
	return true;
}

void releaseId(const string &id)
{
	usedDefinitionIds.erase(id);
}
}

WorkflowDefinition::WorkflowDefinition(string id, string name, int version)
	: id(""), name(name), version(version), root(nullptr)
{
	if (reserveId(id))
	{
		this->id = id;
	}
}

WorkflowDefinition::~WorkflowDefinition()
{
	if (!id.empty())
	{
		releaseId(id);
	}

	if (root)
	{
		delete root;
		root = nullptr;
	}

	for (auto iterator = this->rules.begin(); iterator < this->rules.end(); iterator++)
	{
		DependencyRule *rule = *iterator;
		if (rule)
			delete rule;
	}

	for (size_t i = 0; i < auditors.size(); ++i)
	{
		delete auditors[i];
	}
	auditors.clear();
}

string WorkflowDefinition::getId()
{
	return id;
}

bool WorkflowDefinition::setId(string newId)
{
	if (newId == id && !id.empty())
	{
		return true;
	}
	if (!reserveId(newId))
	{
		return false;
	}
	if (!id.empty())
	{
		releaseId(id);
	}
	id = newId;
	return true;
}

string WorkflowDefinition::getName()
{
	return name;
}

string WorkflowDefinition::getDescription()
{
	return description;
}

void WorkflowDefinition::setDescription(string description)
{
	this->description = description;
}

void WorkflowDefinition::setName(string name)
{
	this->name = name;
}

int WorkflowDefinition::getVersion()
{
	return version;
}

void WorkflowDefinition::setVersion(int version)
{
	this->version = version;
}

WorkItem *WorkflowDefinition::getRoot()
{
	return root;
}

void WorkflowDefinition::setRoot(WorkItem *root)
{
	if (this->root == root)
	{
		return;
	}
	WorkItemDecorator *decorator = dynamic_cast<WorkItemDecorator *>(root);
	if (decorator != nullptr && decorator->getInner() == this->root)
	{
		this->root = root;
		return;
	}
	delete this->root;
	this->root = root;
}

vector<DependencyRule *> WorkflowDefinition::getRules()
{
	return rules;
}

void WorkflowDefinition::addRule(DependencyRule *newRule)
{
	if (newRule)
		this->rules.push_back(newRule);
}

void WorkflowDefinition::addAuditLogger(AuditLogger *logger)
{
	if (logger)
		auditors.push_back(logger);
}

WorkItem *WorkflowDefinition::instantiateTree()
{
	if (root == nullptr)
		return nullptr;
	return root->clone();
}

void WorkflowDefinition::describe()
{
	cout << getId() << "|" << getName() << "|" << getVersion() << "\n";
	if (root == nullptr)
	{
		cout << "(no root)\n";
	}
	else
	{
		set<WorkItem *> seen;
		describeItem(root, 0, seen);
	}
	cout << "DEPENDENCIES\n";
	for (size_t i = 0; i < rules.size(); ++i)
	{
		DependencyRule *rule = rules[i];
		if (rule == nullptr)
		{
			continue;
		}
		cout << rule->fromId << "|" << rule->toId << "|" << rule->outcome << "\n";
	}
}
