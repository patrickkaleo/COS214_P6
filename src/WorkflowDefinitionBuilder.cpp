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

void WorkflowDefinitionBuilder::addProcess(
		string id, string name, string parentId)
{
	if (definition == nullptr)
		return;

	SubProcess *process = new SubProcess(id, name);

	// Root process has no parent.
	if (parentId == "-")
	{
		definition->setRoot(process);
		items[id] = process;
		return;
	}

	SubProcess *parent =
			dynamic_cast<SubProcess *>(findItem(parentId));

	if (parent == nullptr)
	{
		delete process;
		return;
	}

	parent->add(process);
	items[id] = process;
}

void WorkflowDefinitionBuilder::addTask(
		string id, string name, string parentId)
{
	if (definition == nullptr)
		return;

	SubProcess *parent =
			dynamic_cast<SubProcess *>(findItem(parentId));

	if (parent == nullptr)
		return;

	Task *task = new Task(id, name);

	parent->add(task);

	// Register task so other build steps can find it.
	items[id] = task;
}

void WorkflowDefinitionBuilder::addDependency(string fromId, string toId, string outcome)
{
	if (definition)
	{
		definition->addRule(new DependencyRule(fromId, toId, outcome));
	}
}

void WorkflowDefinitionBuilder::setRouting(
		string itemId, RoutingStrategy *strategy)
{
	Task *task =
			dynamic_cast<Task *>(findItem(itemId));

	if (task == nullptr)
	{
		delete strategy;
		return;
	}

	task->setRouting(strategy);
}

void WorkflowDefinitionBuilder::setService(
		string itemId,
		string serviceName)
{
	Task *task = dynamic_cast<Task *>(findItem(itemId));

	if (task == nullptr)
		return;

	task->setServiceName(serviceName);
}

void WorkflowDefinitionBuilder::markCheckpoint(
		string itemId)
{
	Task *task =
			dynamic_cast<Task *>(findItem(itemId));

	if (task == nullptr)
		return;

	task->setCheckpoint(true);
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
