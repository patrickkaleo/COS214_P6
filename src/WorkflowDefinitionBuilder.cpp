#include "WorkflowDefinitionBuilder.h"
#include "WorkItemDecorator.h"
#include <functional>
#include <memory>
#include <set>
#include <stdexcept>

namespace {
WorkItem *unwrapped(WorkItem *item) {
	set<WorkItem *> layers;
	while (WorkItemDecorator *layer = dynamic_cast<WorkItemDecorator *>(item)) {
		if (!layers.insert(item).second) throw invalid_argument("Decorator cycle in workflow tree");
		item = layer->getInner();
	}
	return item;
}

void requireItemIdentity(const string &id, const string &name) {
	if (id.empty() || id == "-" || name.empty())
		throw invalid_argument("Work items require a nonempty id and name; '-' is reserved");
}
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
	if (definition) validateDefinition(definition);
	WorkflowDefinition *result = definition;
	definition = nullptr;
	items.clear();
	return result;
}

void WorkflowDefinitionBuilder::validateDefinition(WorkflowDefinition *product)
{
	if (!product || !product->getRoot())
		throw invalid_argument("Workflow definition requires a root process");
	if (product->getName().empty() || product->getVersion() < 1)
		throw invalid_argument("Workflow definition requires a name and positive version");
	if (!dynamic_cast<SubProcess *>(unwrapped(product->getRoot())))
		throw invalid_argument("Workflow root must be a process");

	map<string, size_t> indices;
	vector<WorkItem *> nodes;
	set<WorkItem *> cores;
	vector<pair<size_t, size_t> > ownership;
	function<void(WorkItem *, SubProcess *, size_t)> visit =
		[&](WorkItem *item, SubProcess *parent, size_t parentIndex) {
			WorkItem *core = unwrapped(item);
			if (!core || !cores.insert(core).second)
				throw invalid_argument("Workflow tree contains a null, shared or cyclic item");
			requireItemIdentity(item->getId(), item->getName());
			const size_t index = nodes.size();
			if (!indices.insert(make_pair(item->getId(), index)).second)
				throw invalid_argument("Duplicate work item id: " + item->getId());
			if (item->getParent() != parent)
				throw invalid_argument("Incorrect parent for work item: " + item->getId());
			nodes.push_back(item);
			if (parent) ownership.push_back(make_pair(parentIndex, index));
			if (SubProcess *process = dynamic_cast<SubProcess *>(core))
				for (int i = 0; i < process->childCount(); ++i)
					visit(process->getChild(i), process, index);
		};
	visit(product->getRoot(), nullptr, 0);

	// Separate activation and completion nodes capture both explicit dependency
	// cycles and deadlocks such as a process waiting on one of its own children.
	vector<vector<size_t> > graph(nodes.size() * 2);
	for (size_t i = 0; i < nodes.size(); ++i) graph[i * 2].push_back(i * 2 + 1);
	for (size_t i = 0; i < ownership.size(); ++i) {
		const size_t parent = ownership[i].first, child = ownership[i].second;
		graph[parent * 2].push_back(child * 2);
		graph[child * 2 + 1].push_back(parent * 2 + 1);
	}
	set<string> rulesSeen;
	const vector<DependencyRule *> rules = product->getRules();
	for (size_t i = 0; i < rules.size(); ++i) {
		const DependencyRule *rule = rules[i];
		if (!rule || indices.count(rule->fromId) == 0 || indices.count(rule->toId) == 0 || rule->outcome.empty())
			throw invalid_argument("Dependency requires known source/target ids and an outcome");
		if (rule->fromId == rule->toId)
			throw invalid_argument("Self dependency on item: " + rule->fromId);
		const string key = rule->fromId + '\0' + rule->toId + '\0' + rule->outcome;
		if (!rulesSeen.insert(key).second) throw invalid_argument("Duplicate dependency rule");
		graph[indices[rule->fromId] * 2 + 1].push_back(indices[rule->toId] * 2);
	}
	vector<int> colour(graph.size(), 0);
	function<void(size_t)> checkCycle = [&](size_t node) {
		if (colour[node] == 1) throw invalid_argument("Dependency or process hierarchy creates a cycle");
		if (colour[node] == 2) return;
		colour[node] = 1;
		for (size_t i = 0; i < graph[node].size(); ++i) checkCycle(graph[node][i]);
		colour[node] = 2;
	};
	for (size_t i = 0; i < graph.size(); ++i) checkCycle(i);
}

void WorkflowDefinitionBuilder::startDefinition(string id, string name, int version)
{
	if (definition) throw invalid_argument("A definition is already being built");
	if (id.empty() || name.empty() || version < 1)
		throw invalid_argument("Definition requires id, name and positive version");
	items.clear();
	definition = new WorkflowDefinition(id, name, version);
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
	if (!definition) throw invalid_argument("Start a definition before adding processes");
	requireItemIdentity(id, name);
	if (findItem(id)) throw invalid_argument("Duplicate work item id: " + id);

	// Root process has no parent.
	if (parentId == "-")
	{
		if (definition->getRoot()) throw invalid_argument("A workflow may have only one root process");
		SubProcess *process = new SubProcess(id, name);
		definition->setRoot(process);
		items[id] = process;
		return;
	}

	SubProcess *parent =
			dynamic_cast<SubProcess *>(unwrapped(findItem(parentId)));

	if (parent == nullptr)
	{
		throw invalid_argument("Unknown process parent: " + parentId);
	}

	SubProcess *process = new SubProcess(id, name);
	parent->add(process);
	items[id] = process;
}

void WorkflowDefinitionBuilder::addTask(
		string id, string name, string parentId)
{
	if (!definition) throw invalid_argument("Start a definition before adding tasks");
	requireItemIdentity(id, name);
	if (findItem(id)) throw invalid_argument("Duplicate work item id: " + id);

	SubProcess *parent =
			dynamic_cast<SubProcess *>(unwrapped(findItem(parentId)));

	if (parent == nullptr)
		throw invalid_argument("Unknown process parent: " + parentId);

	Task *task = new Task(id, name);

	parent->add(task);

	// Register task so other build steps can find it.
	items[id] = task;
}

void WorkflowDefinitionBuilder::addDependency(string fromId, string toId, string outcome)
{
	if (!definition || !findItem(fromId) || !findItem(toId) || outcome.empty())
		throw invalid_argument("Dependency requires known source/target ids and an outcome");
	if (fromId == toId) throw invalid_argument("Self dependency on item: " + fromId);
	definition->addRule(new DependencyRule(fromId, toId, outcome));
}

void WorkflowDefinitionBuilder::setRouting(
		string itemId, RoutingStrategy *strategy)
{
	unique_ptr<RoutingStrategy> owned(strategy);
	Task *task =
			dynamic_cast<Task *>(unwrapped(findItem(itemId)));

	if (!task || !strategy) throw invalid_argument("Routing requires a known task and strategy: " + itemId);

	task->setRouting(owned.release());
}

void WorkflowDefinitionBuilder::setService(
		string itemId,
		string serviceName)
{
	Task *task = dynamic_cast<Task *>(unwrapped(findItem(itemId)));

	if (!task || serviceName.empty()) throw invalid_argument("Service requires a known task and name: " + itemId);

	task->setServiceName(serviceName);
}

void WorkflowDefinitionBuilder::markCheckpoint(
		string itemId)
{
	Task *task =
			dynamic_cast<Task *>(unwrapped(findItem(itemId)));

	if (task == nullptr)
		throw invalid_argument("Checkpoint requires a known task: " + itemId);

	task->setCheckpoint(true);
}

void WorkflowDefinitionBuilder::addPriority(string itemId, int level)
{
	WorkItem *item = findItem(itemId);
	if (!item || level < 0) throw invalid_argument("Priority requires a known item and nonnegative level: " + itemId);

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

void WorkflowDefinitionBuilder::setDeadlineHours(string itemId, int hours)
{
	Task *task = dynamic_cast<Task *>(unwrapped(findItem(itemId)));
	if (!task || hours < 1 || hours > 87600)
		throw invalid_argument("Deadline requires a known task and 1..87600 hours: " + itemId);
	task->setDeadlineHours(hours);
}

void WorkflowDefinitionBuilder::addAudit(string itemId)
{
	if (definition == nullptr || definition->getRoot() == nullptr)
		throw invalid_argument("Audit requires a workflow tree");

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
		throw invalid_argument("Audit requires a known item: " + itemId);

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
	throw invalid_argument("Could not attach audit decorator to item: " + itemId);
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
	throw invalid_argument("Could not attach decorator to workflow tree");
}
