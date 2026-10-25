#include "WorkflowDefintion.h"

WorkflowDefinition::WorkflowDefinition(string id, string name, int version)
	: id(id), name(name), version(version), root(nullptr)
{
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

SubProcess* WorkflowDefinition::getRoot()
{
	return root;
}

void WorkflowDefinition::setRoot(SubProcess* root)
{
	if (this->root != root)
	{
		delete this->root;
		this->root = root;
	}
}

vector<DependencyRule>& WorkflowDefinition::getRules()
{
	return rules;
}

void WorkflowDefinition::addRule(string fromId, string toId, string outcome)
{
	rules.push_back(DependencyRule(fromId, toId, outcome));
}

SubProcess* WorkflowDefinition::instantiateTree()
{
	return this->root->clone();
}
