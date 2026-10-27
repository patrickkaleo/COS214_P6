#include "WorkflowDefinitionBuilder.h"
WorkflowDefinitionBuilder::~WorkflowDefinitionBuilder()
{
	if (this->definition)
	{
		delete definition;
		this->definition = nullptr;
	}
}

WorkflowDefinition *WorkflowDefinitionBuilder::getResult()
{
	WorkflowDefinition *result = definition;
	definition = nullptr;
	return result;
}

void WorkflowDefinitionBuilder::startDefinition(string id, string name, int version)
{
	if (definition == nullptr)
	{
		definition = new WorkflowDefinition(id, name, version);
	}
}

WorkItem *WorkflowDefinitionBuilder::findItem(string itemId)
{
	map<string, WorkItem *>::iterator found = items.find(itemId);
	if (found == items.end())
	{
		return nullptr;
	}
	return found->second;
}

void WorkflowDefinitionBuilder::addProcess(string id, string name, string parentId)
{
	if (definition)
	{
		WorkItem *parent = findItem(parentId);
		if (parent)
			parent->add(new SubProcess(id, name));
	}
}

void WorkflowDefinitionBuilder::addTask(string id, string name, string parentId)
{
	if (definition)
	{
		WorkItem *parent = findItem(parentId);
		if (parent)
			parent->add(new Task(id, name));
	}
}

void WorkflowDefinitionBuilder::addDependency(string fromId, string toId, string outcome)
{
	if (definition)
	{
		definition->addRule(new DependencyRule(fromId, toId, outcome));
	}
}

void WorkflowDefinitionBuilder::setRouting(string itemId, RoutingStrategy *strategy)
{
	if (strategy)
	{
		WorkItem *item = findItem(itemId);
		if (item)
			item->setRouting(strategy);
	}
}

void WorkflowDefinitionBuilder::setService(string itemId, string serviceName)
{
	WorkItem *item = findItem(itemId);
	if (item)
	{
		item->setService(serviceName);
	}
}

void WorkflowDefinitionBuilder::markCheckpoint(string itemId)
{
	WorkItem *item = findItem(itemId);
	if (item != nullptr)
	{
		item->setCheckointflag(true);
	}
}

void WorkflowDefinitionBuilder::addPriority(string itemId, int level)
{
	WorkItem *item = findItem(itemId);
	if (item)
	{
		WorkItem *wrapped = new PriorityDecorator(item, level);
		replaceItem(item, wrapped);
		items[itemId] = wrapped;
	}
}

void WorkflowDefinitionBuilder::addAudit(string itemId)
{
	WorkItem *item = findItem(itemId);
	if (item)
	{
		AuditLogger *auditor = new AuditLogger(false);
		WorkItem *wrapped = new AuditDecorator(item, auditor);
		replaceItem(item, wrapped);
		items[itemId] = wrapped;
	}
}

void WorkflowDefinitionBuilder::replaceItem(WorkItem *&current, WorkItem *replacement)
{
	current = replacement;
}
