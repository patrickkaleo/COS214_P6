#include "WorkflowDefinitionBuilder.h"

WorkflowDefinitionBuilder::WorkflowDefinitionBuilder(string id, string name, int version)
	: definition(new WorkflowDefinition(id, name, version))
{
}

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

void WorkflowDefinitionBuilder::addWorkItem(WorkItem *newWorkItem, WorkItem *parent)
{
	if (parent && newWorkItem)
	{
		parent->add(newWorkItem);
	}
}

void WorkflowDefinitionBuilder::addDependency(DependencyRule *newDependencyRule)
{
	if (definition)
		definition->addRule(newDependencyRule);
}

void WorkflowDefinitionBuilder::setRouting(WorkItem *item, RoutingStrategy *strategy)
{
	if (item && strategy)
		item->setRouting(strategy);
}

void WorkflowDefinitionBuilder::setService(WorkItem *item, string serviceName)
{
	if (item)
		item->setService(serviceName);
}

void WorkflowDefinitionBuilder::markCheckpoint(WorkItem *item)
{
	if (item)
		item->
}

void WorkflowDefinitionBuilder::addPriority(WorkItem *item, int level)
{
	if (level >= 0 && item)
		item = new PriorityDecorator(item, level);
}

void WorkflowDefinitionBuilder::addAudit(WorkItem *item)
{
	if (item)
		item = new AuditDecorator();
}
