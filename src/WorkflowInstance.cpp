#include "WorkflowInstance.h"
#include "CreatedState.h"
#include "AvailableState.h"
#include "AssignedState.h"
#include "InProgressState.h"
#include "EscalatedState.h"
#include "CompletedState.h"
#include "RejectedState.h"
#include "CancelledState.h"
#include "Task.h"
#include "WorkItemDecorator.h"
#include "Participant.h"
#include "ParticipantDirectory.h"
#include "RoutingStrategy.h"
#include "ApprovalRouting.h"
#include "DepthFirstWorkItemIterator.h"
#include <memory>
#include <limits>
#include <cstdint>

namespace {
WorkItem *coreOf(WorkItem *item) {
    while (WorkItemDecorator *decorator = dynamic_cast<WorkItemDecorator *>(item)) {
        item = decorator->getInner();
    }
    return item;
}

std::vector<DependencyRule> definitionRules(const WorkflowDefinition *definition) {
    std::vector<DependencyRule> result;
    if (definition) {
        for (DependencyRule *rule : const_cast<WorkflowDefinition *>(definition)->getRules()) {
            if (rule) result.push_back(*rule);
        }
    }
    return result;
}
}

WorkflowInstance::WorkflowInstance(const std::string &id,
                                   const WorkflowDefinition *definition,
                                   const InstanceData &initial)
    : WorkflowInstance(id, definition, initial, nullptr, nullptr) {}

WorkflowInstance::WorkflowInstance(const std::string &id,
                                   const WorkflowDefinition *definition,
                                   const InstanceData &initial,
                                   ParticipantDirectory *directory,
                                   ServiceRegistry *services)
    : Id(id), definition(definition), mediator(new DependencyMediator(definitionRules(definition))),
      rootProcess(nullptr), data(initial), participants(directory) {
    if (definition) {
        rootProcess = const_cast<WorkflowDefinition *>(definition)->instantiateTree();
    }
    wireRuntime(services);
}

WorkflowInstance::WorkflowInstance(const std::string &id,
                                   const WorkflowDefinition *definition,
                                   SubProcess *root,
                                   DependencyMediator *instanceMediator,
                                   const InstanceData &initial,
                                   ParticipantDirectory *directory)
    : Id(id), definition(definition),
      mediator(instanceMediator ? instanceMediator : new DependencyMediator(definitionRules(definition))),
      rootProcess(root), data(initial), participants(directory) {
    wireRuntime(nullptr);
}

void WorkflowInstance::wireRuntime(ServiceRegistry *services) {
    if (!rootProcess) return;
    rootProcess->setRoutingContext(&data, participants);
    DepthFirstWorkItemIterator it(rootProcess);
    for (it.first(); !it.isDone(); it.next()) {
        WorkItem *item = it.current();
        item->setInstanceId(Id);
        if (Task *task = dynamic_cast<Task *>(coreOf(item))) {
            task->setServiceRegistry(services);
            // The builder stores priority in this level. A new instance has had no escalations,
            // and an approval chain must start from its first level.
            task->setEscalationLevel(0);
        }
    }
    mediator->registerTree(*rootProcess);
}

WorkflowInstance::~WorkflowInstance() {
    delete rootProcess;
    delete mediator;
}

bool WorkflowInstance::start() {
    if (!rootProcess || !dynamic_cast<SubProcess *>(coreOf(rootProcess))) return false;
    const bool started = rootProcess->makeAvailable();
    if (started) refreshDeadlines(std::time(nullptr));
    return started;
}

const std::string &WorkflowInstance::getId() const { return Id; }
InstanceData &WorkflowInstance::getData() { return data; }
const InstanceData &WorkflowInstance::getData() const { return data; }
WorkItem *WorkflowInstance::getRoot() const { return rootProcess; }
std::string WorkflowInstance::getDefinitionId() const {
    return definition ? const_cast<WorkflowDefinition *>(definition)->getId() : "";
}

WorkItem *WorkflowInstance::findItem(const std::string &itemId) const {
    return findItemRecursive(rootProcess, itemId);
}

WorkItem *WorkflowInstance::findItemRecursive(WorkItem *current, const std::string &itemId) const {
    if (!current) return nullptr;
    if (current->getId() == itemId) return current;
    for (int i = 0; i < current->childCount(); ++i) {
        if (WorkItem *found = findItemRecursive(current->getChild(i), itemId)) return found;
    }
    return nullptr;
}

std::string WorkflowInstance::status() const {
    if (!rootProcess) return "invalid";
    const std::string state = rootProcess->getStateName();
    if (state == "Created") return "created";
    if (state == "Completed") return "completed";
    if (state == "Rejected" || state == "Cancelled") return "terminated";
    return "running";
}

bool WorkflowInstance::isRegisteredParticipant(const Participant *actor) const {
    return actor && participants && participants->findById(actor->getId()).get() == actor;
}

bool WorkflowInstance::canAssign(const WorkItem &item, const Participant *actor) const {
    if (!isRegisteredParticipant(actor)) return false;
    Task *task = dynamic_cast<Task *>(coreOf(const_cast<WorkItem *>(&item)));
    if (!task) return false;
    if (!task->getRouting()) return true;

    // Evaluate the existing strategy with this candidate alone. This preserves
    // role/data/approval policy while allowing a qualified second team member.
    ParticipantDirectory candidateDirectory;
    candidateDirectory.addParticipant(std::make_shared<Participant>(*actor));
    std::unique_ptr<RoutingStrategy> strategy(task->getRouting()->clone());
    if (!strategy || !strategy->select(*task, data, candidateDirectory)) return false;
    if (ApprovalRouting *candidate = dynamic_cast<ApprovalRouting *>(strategy.get())) {
        std::unique_ptr<RoutingStrategy> current(task->getRouting()->clone());
        if (!current->select(*task, data, *participants)) return false;
        const ApprovalRouting *required = static_cast<const ApprovalRouting *>(current.get());
        return candidate->getSelectedLevel() == required->getSelectedLevel();
    }
    return true;
}

bool WorkflowInstance::canAct(const WorkItem &item, const Participant *actor,
                             const std::string &action) const {
    if (!isRegisteredParticipant(actor) || item.isFinished()) return false;
    const std::string state = item.getStateName();
    const bool isTask = dynamic_cast<Task *>(coreOf(const_cast<WorkItem *>(&item))) != nullptr;
    const bool assigned = item.getAssignee() == actor;
    if (action == "claim") {
        return isTask && (state == "Available" || state == "Escalated") && canAssign(item, actor);
    }
    if (action == "start") return isTask && state == "Assigned" && assigned;
    if (action == "complete" || action == "reject") {
        return isTask && state == "InProgress" && assigned;
    }
    if (action == "delegate") return isTask && state == "Assigned" && assigned;
    if (action == "cancel") {
        if (!isTask) return actor->hasRole("WorkflowAdmin");
        return assigned || (!item.getAssignee() && state != "Created" && canAssign(item, actor));
    }
    return false;
}

std::vector<std::string> WorkflowInstance::availableActions(const std::string &itemId,
                                                           const Participant *actor) const {
    std::vector<std::string> result;
    WorkItem *item = findItem(itemId);
    if (!item) return result;
    const char *actions[] = {"claim", "start", "complete", "reject", "delegate", "cancel"};
    for (const char *action : actions) {
        if (canAct(*item, actor, action)) result.push_back(action);
    }
    return result;
}

void WorkflowInstance::captureItem(WorkItem *item,
                                    std::map<std::string, std::string> &states,
                                    std::map<std::string, std::string> &assignees,
                                    std::map<std::string, std::string> &outcomes) const {
    if (!item) return;
    states[item->getId()] = item->getStateName();
    outcomes[item->getId()] = item->getOutcome();
    if (item->getAssignee()) assignees[item->getId()] = item->getAssignee()->getId();
    for (int i = 0; i < item->childCount(); ++i) {
        captureItem(item->getChild(i), states, assignees, outcomes);
    }
}

WorkflowSnapshot *WorkflowInstance::createSnapshot(const std::string &label) {
    std::map<std::string, std::string> states, assignees, outcomes;
    captureItem(rootProcess, states, assignees, outcomes);
    WorkflowSnapshot *snapshot = new WorkflowSnapshot(states, assignees, outcomes, data, label);
    DepthFirstWorkItemIterator it(rootProcess);
    for (it.first(); !it.isDone(); it.next()) {
        WorkItem *item = it.current();
        snapshot->escalationLevels[item->getId()] = item->getEscalationLevel();
    }
    if (mediator) {
        snapshot->openedBranches = mediator->getOpenedBranches();
        snapshot->decidedItems = mediator->getDecidedItems();
    }
    snapshot->deadlines = deadlines;
    snapshot->deadlineOwners = deadlineOwners;
    return snapshot;
}

void WorkflowInstance::restoreSnapshot(const WorkflowSnapshot &snapshot) {
    data = snapshot.data;
    clearAssignees(rootProcess);
    for (const auto &entry : snapshot.itemStates) {
        WorkItem *item = findItem(entry.first);
        if (!item) continue;
        if (WorkItemState *state = createStateFromName(entry.second)) item->setState(state);
    }
    for (const auto &entry : snapshot.outcomes) {
        if (WorkItem *item = findItem(entry.first)) item->setOutcome(entry.second);
    }
    for (const auto &entry : snapshot.escalationLevels) {
        if (WorkItem *item = findItem(entry.first)) item->setEscalationLevel(entry.second);
    }
    if (participants) {
        for (const auto &entry : snapshot.assignees) {
            if (WorkItem *item = findItem(entry.first)) {
                item->setAssignee(participants->findById(entry.second).get());
            }
        }
    }
    if (mediator) mediator->restoreBookkeeping(snapshot.openedBranches, snapshot.decidedItems);
    deadlines = snapshot.deadlines;
    deadlineOwners = snapshot.deadlineOwners;
}

void WorkflowInstance::clearAssignees(WorkItem *item) {
    if (!item) return;
    item->setAssignee(nullptr);
    for (int i = 0; i < item->childCount(); ++i) clearAssignees(item->getChild(i));
}

void WorkflowInstance::refreshDeadlines(std::time_t now) {
    DepthFirstWorkItemIterator it(rootProcess);
    for (it.first(); !it.isDone(); it.next()) {
        WorkItem *item = it.current();
        Task *task = dynamic_cast<Task *>(coreOf(item));
        const std::string id = item->getId();
        const std::string state = item->getStateName();
        if (!task || !item->getAssignee() || (state != "Assigned" && state != "InProgress")) {
            deadlines.erase(id);
            deadlineOwners.erase(id);
            continue;
        }
        const std::string owner = item->getAssignee()->getId() + ":" +
                                  std::to_string(item->getEscalationLevel());
        if (deadlineOwners[id] == owner) continue;
        deadlines.erase(id);
        deadlineOwners[id] = owner;
        const int hours = task->getDeadlineHours();
        if (hours > 0) {
            const std::uintmax_t seconds = static_cast<std::uintmax_t>(hours) * 3600u;
            const std::time_t maximum = std::numeric_limits<std::time_t>::max();
            if (seconds <= static_cast<std::uintmax_t>(maximum)) {
                const std::time_t duration = static_cast<std::time_t>(seconds);
                if (now <= maximum - duration) deadlines[id] = now + duration;
            }
        }
    }
}

void WorkflowInstance::checkDeadlines(std::time_t now) {
    refreshDeadlines(now);
    const std::map<std::string, std::time_t> pending = deadlines;
    for (const auto &entry : pending) {
        if (now < entry.second) continue;
        WorkItem *item = findItem(entry.first);
        Task *task = dynamic_cast<Task *>(coreOf(item));
        if (!task || item->isFinished()) continue;
        deadlines.erase(entry.first);
        deadlineOwners.erase(entry.first);
        // An approval chain stops at its top: nobody above can take it, so the holder keeps it.
        if (dynamic_cast<ApprovalRouting *>(task->getRouting())) {
            const int level = item->getEscalationLevel();
            std::unique_ptr<RoutingStrategy> probe(task->getRouting()->clone());
            item->setEscalationLevel(level + 1);
            const bool higherAuthority = probe->select(*task, data, *participants) != nullptr;
            item->setEscalationLevel(level);
            if (!higherAuthority) continue;
        }
        if (item->escalate() && dynamic_cast<ApprovalRouting *>(task->getRouting())) task->route();
    }
    refreshDeadlines(now);
}

std::time_t WorkflowInstance::getDeadline(const std::string &itemId) const {
    const auto found = deadlines.find(itemId);
    return found == deadlines.end() ? 0 : found->second;
}

WorkItemState *WorkflowInstance::createStateFromName(const std::string &name) const {
    if (name == "Created") return new CreatedState();
    if (name == "Available") return new AvailableState();
    if (name == "Assigned") return new AssignedState();
    if (name == "InProgress") return new InProgressState();
    if (name == "Escalated") return new EscalatedState();
    if (name == "Completed") return new CompletedState();
    if (name == "Rejected") return new RejectedState();
    if (name == "Cancelled") return new CancelledState();
    return nullptr;
}
