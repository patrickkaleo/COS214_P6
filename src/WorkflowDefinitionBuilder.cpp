#include "WorkflowDefinitionBuilder.h"


WorkflowDefinition* WorkflowDefinitionBuilder::getResult()
{
	WorkflowDefinition* result = definition;
	definition = nullptr;
	return result;
}

void WorkflowDefinitionBuilder::addSubProcess(string id, string name, string parentId)
{
	if (definition == nullptr)
	{
		return;
	}

	SubProcess* created = new SubProcess(id, name);
	if (parentId.empty())
	{
		definition->setRoot(created);
		return;
	}

	SubProcess* parent = findProcess(definition->getRoot(), parentId);
	if (parent == nullptr)
	{
		delete created;
		return;
	}
	parent->add(created);
}

void WorkflowDefinitionBuilder::addTask(string id, string name, string parentId)
{
	if (definition == nullptr)
	{
		return;
	}

	Task* task = new Task(id, name);
	SubProcess* parent = findProcess(definition->getRoot(), parentId);
	if (parent == nullptr)
	{
		delete task;
		return;
	}
	parent->add(task);
}

void WorkflowDefinitionBuilder::addDependency(string fromId, string toId, string outcome)
{
	if (definition == nullptr)
	{
		return;
	}
	definition->addRule(fromId, toId, outcome);
}

void WorkflowDefinitionBuilder::setRouting(string itemId, RoutingStrategy* strategy)
{
	if (definition == nullptr || definition->getRoot() == nullptr)
		return;

		
	// find the item, and set its routing stretegy to the strategy 
	
}

void WorkflowDefinitionBuilder::setService(string itemId, string serviceName)
{
	if (definition == nullptr || definition->getRoot() == nullptr)
	{
		return;
	}

	// fint the work item and set a service on that 
	task->setServiceName(serviceName);
}

void WorkflowDefinitionBuilder::markCheckpoint(string itemId)
{
	if (definition == nullptr || definition->getRoot() == nullptr)
	{
		return;
	}

	// mark checkopoint on an item with the id
	// task->setCheckpoint(true);
}

void WorkflowDefinitionBuilder::addPriority(string itemId, int level)
{
	if (definition == nullptr || definition->getRoot() == nullptr)
	{
		return;
	}

	// priority decoration for a given item with the id 
}

void WorkflowDefinitionBuilder::addAudit(string itemId)
{
	if (definition == nullptr || definition->getRoot() == nullptr)
	{
		return;
	}

	SubProcess* parent = nullptr;
	WorkItem* item = findItem(definition->getRoot(), itemId, parent);
	if (item == nullptr || parent == nullptr)
	{
		return;
	}

	AuditLogger* auditor = new AuditLogger();
	replaceChild(parent, item, new AuditDecorator(item, auditor));
}
