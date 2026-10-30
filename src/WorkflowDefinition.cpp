#include "WorkflowDefinition.h"
#include "WorkItemDecorator.h"

#include <set>

namespace
{
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

WorkItem *WorkflowDefinition::instantiateTree()
{
	if (root == nullptr)
		return nullptr;
	return root->clone();
}
