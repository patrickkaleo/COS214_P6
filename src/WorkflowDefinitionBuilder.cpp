#include "WorkflowDefinitionBuilder.h"
#include "WorkItemDecorator.h"
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
	if (item == nullptr)
		return;

	WorkItem *core = item;
	while (WorkItemDecorator *layer = dynamic_cast<WorkItemDecorator *>(core))
	{
		if (layer->getInner() == nullptr)
			break;
		core = layer->getInner();
	}

	Task *task = dynamic_cast<Task *>(core);
	if (task)
	{
		task->setEscalationLevel(level);
	}

	WorkItem *wrapped = new PriorityDecorator(item, level);
	replaceItem(item, wrapped);
}

void WorkflowDefinitionBuilder::addAudit(string itemId)
{
	if (definition == nullptr || definition->getRoot() == nullptr)
		return;

	// Decorate the node that is actually in the tree. After a priority wrap,
	// that node is the priority decorator, and audit goes around it.
	WorkItem *item = findItem(itemId);
	SubProcess *root = dynamic_cast<SubProcess *>(definition->getRoot());
	if (root != nullptr)
	{
		WorkItem *inTree = root->find(itemId);
		if (inTree != nullptr)
			item = inTree;
	}
	if (item == nullptr)
		return;

	// AuditDecorator does not own the logger. The definition deletes it.
	AuditLogger *auditor = new AuditLogger(false);
	definition->addAuditLogger(auditor);

	SubProcess *parent = item->getParent();
	AuditDecorator *decorated = new AuditDecorator(item, auditor);
	bool placed = false;
	if (parent != nullptr)
	{
		placed = parent->replaceChild(item, decorated);
	}
	else if (definition->getRoot() == item)
	{
		definition->setRoot(decorated);
		placed = definition->getRoot() == decorated;
	}

	if (placed)
	{
		items[itemId] = decorated;
		return;
	}

	// The wrapper never joined the tree. Drop it without deleting the item
	// the parent still owns.
	decorated->releaseInner();
	delete decorated;
}

void WorkflowDefinitionBuilder::replaceItem(WorkItem *&current, WorkItem *replacement)
{
	if (current == nullptr || replacement == nullptr || current == replacement)
		return;

	bool placed = false;
	SubProcess *parent = current->getParent();
	if (parent != nullptr)
	{
		placed = parent->replaceChild(current, replacement);
	}
	else if (definition != nullptr && definition->getRoot() == current)
	{
		definition->setRoot(replacement);
		placed = definition->getRoot() == replacement;
	}

	if (placed)
	{
		items[replacement->getId()] = replacement;
		current = replacement;
		return;
	}

	// The wrapper never joined the tree. Drop it without deleting the item
	// the parent or the definition root still owns.
	WorkItemDecorator *shell = dynamic_cast<WorkItemDecorator *>(replacement);
	if (shell != nullptr)
	{
		shell->releaseInner();
	}
	delete replacement;
}
