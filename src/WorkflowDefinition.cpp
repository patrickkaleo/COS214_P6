#include "WorkflowDefintion.h"

WorkflowDefinition::WorkflowDefinition(string id, string name, int version)
	: id(id), name(name), version(version), root(nullptr)
{
}

WorkflowDefinition::~WorkflowDefinition()
{
	delete root;
}

string WorkflowDefinition::getId()
{
	return id;
}

void WorkflowDefinition::setId(string id)
{
	this->id = id;
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
	if (this->root != root)
	{
		delete this->root;
		this->root = root;
	}
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
