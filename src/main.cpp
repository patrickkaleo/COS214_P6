/**
 * @file main.cpp
 * @brief Main interactive Terminal User Interface (TUI) entry point for the TIVIDY Workflow Engine.
 * @details The TUI only talks to TividyWorkflowFacade. Each industry is supplied as data (a
 *          .scenario file naming a .workflow definition, its participants and input fields),
 *          so nothing below knows about any particular industry. Every action stays on offer
 *          even when it is illegal: the engine decides, and the TUI reports the refusal.
 */

#include "AuditLogger.h"
#include "BankGatewayAdapter.h"
#include "CarrierAdapter.h"
#include "GovernmentIDAdapter.h"
#include "MonitoringService.h"
#include "NotificationService.h"
#include "TividyWorkflowFacade.h"
#include "TrackingServiceAdapter.h"
#include "WorkflowEventListener.h"

#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace {

const char *const SCENARIO_FILES[] = {
    "data/education_admissions.scenario",
    "data/logistics_shipment_exception.scenario",
};

/// A participant as listed in a scenario file.
struct Person {
    std::string id;
    std::string name;
    std::string roles;
    std::string industry;
};

/// One value the user supplies when starting an instance.
struct InputField {
    std::string key;
    std::string label;
    std::string fallback;
};

/// One industry: what to tell the user, and what to give the engine.
struct Scenario {
    std::string industry;
    std::string process;
    std::string definitionPath;
    std::string about;
    std::string expected;
    std::string definitionId;
    std::vector<Person> people;
    std::vector<InputField> inputs;
};

/// A running instance and the scenario it was started from.
struct InstanceRecord {
    std::string id;
    size_t scenario;
};

/// One line of an instance's history.
struct HistoryEntry {
    std::string instanceId;
    std::time_t when;
    std::string itemName;
    std::string what;
    std::string detail;
};

/// A successful action, so undo can say what it is reversing.
struct UndoEntry {
    std::string instanceId;
    std::string description;
};

// ---- Formatting -----------------------------------------------------------------------------

bool useColour() {
    static const bool enabled = isatty(STDOUT_FILENO) != 0 && std::getenv("NO_COLOR") == nullptr;
    return enabled;
}

std::string paint(const std::string &text, const char *code) {
    return useColour() ? "\033[" + std::string(code) + "m" + text + "\033[0m" : text;
}

std::string bold(const std::string &text) { return paint(text, "1"); }
std::string dim(const std::string &text) { return paint(text, "2"); }
std::string good(const std::string &text) { return paint(text, "32"); }
std::string warn(const std::string &text) { return paint(text, "33"); }
std::string bad(const std::string &text) { return paint(text, "31"); }

std::string pad(const std::string &text, size_t width) {
    return text.size() >= width ? text : text + std::string(width - text.size(), ' ');
}

/// Pads first, because colour codes would otherwise count towards the width.
std::string stateCell(const std::string &state, size_t width) {
    const std::string cell = pad(state, width);
    if (state == "Completed") return good(cell);
    if (state == "Rejected" || state == "Cancelled") return bad(cell);
    if (state == "Escalated") return warn(cell);
    if (state == "Created") return dim(cell);
    return paint(cell, "36");
}

/// An outcome only means something once work has finished or escalated; a retry may leave an old one behind.
std::string shownOutcome(const WorkItemInfo &item) {
    const bool settled = item.state == "Completed" || item.state == "Rejected" || item.state == "Cancelled" || item.state == "Escalated";
    return settled ? item.outcome : "";
}

std::string statusText(const std::string &status) {
    if (status == "completed") return good(status);
    if (status == "terminated") return bad(status);
    return paint(status, "36");
}

void heading(const std::string &title) {
    std::cout << "\n" << bold(title) << "\n" << dim(std::string(title.size(), '-')) << "\n";
}

std::string clock(std::time_t when) {
    char text[16] = {0};
    std::tm *local = std::localtime(&when);
    if (local != nullptr) {
        std::strftime(text, sizeof(text), "%H:%M:%S", local);
    }
    return text;
}

// ---- Scenario files -------------------------------------------------------------------------

std::string trimmed(const std::string &text) {
    const std::string blanks = " \t\r";
    const size_t first = text.find_first_not_of(blanks);
    if (first == std::string::npos) {
        return "";
    }
    return text.substr(first, text.find_last_not_of(blanks) - first + 1);
}

std::vector<std::string> splitFields(const std::string &line) {
    std::vector<std::string> fields;
    std::istringstream in(line);
    std::string field;
    while (std::getline(in, field, '|')) {
        fields.push_back(trimmed(field));
    }
    return fields;
}

bool loadScenario(const std::string &path, Scenario &scenario) {
    std::ifstream file(path.c_str());
    if (!file) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trimmed(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }
        const std::vector<std::string> fields = splitFields(line);
        const std::string &key = fields[0];
        if (fields.size() == 2 && key == "INDUSTRY") scenario.industry = fields[1];
        else if (fields.size() == 2 && key == "PROCESS") scenario.process = fields[1];
        else if (fields.size() == 2 && key == "DEFINITION") scenario.definitionPath = fields[1];
        else if (fields.size() == 2 && key == "ABOUT") scenario.about = fields[1];
        else if (fields.size() == 2 && key == "EXPECTED") scenario.expected = fields[1];
        else if (fields.size() == 4 && key == "PARTICIPANT") {
            Person person;
            person.id = fields[1];
            person.name = fields[2];
            person.roles = fields[3];
            scenario.people.push_back(person);
        } else if (fields.size() == 4 && key == "INPUT") {
            InputField input;
            input.key = fields[1];
            input.label = fields[2];
            input.fallback = fields[3];
            scenario.inputs.push_back(input);
        }
    }

    for (size_t i = 0; i < scenario.people.size(); ++i) {
        scenario.people[i].industry = scenario.industry;
    }
    return !scenario.definitionPath.empty();
}

std::vector<std::string> splitRoles(const std::string &roles) {
    std::vector<std::string> result;
    std::istringstream in(roles);
    std::string role;
    while (std::getline(in, role, ',')) {
        result.push_back(trimmed(role));
    }
    return result;
}

// ---- Observers ------------------------------------------------------------------------------

/**
 * @class EventQueue
 * @brief Observer that holds events until the TUI can show them after the current action.
 *
 * One action can cause several events (a completion opens the next items and routes them),
 * and the assignee of a routed item is only worth showing once that has all happened.
 */
class EventQueue : public WorkflowEventListener {
public:
    void onEvent(const WorkflowEvent &event) override { pending.push_back(event); }

    bool empty() const { return pending.empty(); }

    std::vector<WorkflowEvent> take() {
        std::vector<WorkflowEvent> events;
        events.swap(pending);
        return events;
    }

private:
    std::vector<WorkflowEvent> pending;
};

// ---- The interface --------------------------------------------------------------------------

/**
 * @class Tui
 * @brief Menu loop over the workflow facade.
 */
class Tui {
public:
    bool initialise();
    void run();

private:
    TividyWorkflowFacade facade;
    EventQueue *events = nullptr;       // owned by the engine
    AuditLogger *audit = nullptr;       // owned by the engine
    MonitoringService *monitor = nullptr; // owned by the engine
    NotificationService *notifier = nullptr; // owned by the engine

    std::vector<Scenario> scenarios;
    std::vector<Person> people;
    std::vector<InstanceRecord> instances;
    std::vector<HistoryEntry> history;
    std::vector<UndoEntry> undoStack;
    size_t actor = 0;
    bool inputClosed = false;

    // Input
    bool ask(const std::string &prompt, std::string &answer);
    int askNumber(const std::string &prompt, int first, int last);

    // Screens
    void showIndustries();
    void startInstanceScreen();
    void openInstanceScreen();
    void instanceScreen(const InstanceRecord &record);
    void itemScreen(const InstanceRecord &record, const std::string &itemId);
    void myWorkScreen();
    void chooseActor();
    void undoLast();
    void showInstanceHistory(const std::string &instanceId);
    void showInstanceData(const std::string &instanceId);
    void showAuditTrail();
    void showMetrics();
    void checkDeadlines();

    // Helpers
    void printWorkTable(const std::vector<WorkItemInfo> &items);
    void printItem(const WorkItemInfo &item);
    void printRoster();
    void flushEvents();
    void perform(const InstanceRecord &record, const WorkItemInfo &item, int choice);
    std::string describeRefusal(const WorkItemInfo &item, int choice) const;
    bool findItem(const std::string &instanceId, const std::string &itemId, WorkItemInfo &item);
    size_t personIndex(const std::string &id) const;
    Participant *currentActor();
    std::string actorLine() const;
};

bool Tui::initialise() {
    events = new EventQueue();
    audit = new AuditLogger(false);
    monitor = new MonitoringService();
    notifier = new NotificationService("Console", false);
    facade.addListener(events);
    facade.addListener(audit);
    facade.addListener(monitor);
    facade.addListener(notifier);

    // Adapter: each industry's definitions refer to these by name only.
    facade.addService(new GovernmentIDAdapter(nullptr));
    facade.addService(new BankGatewayAdapter(nullptr));
    facade.addService(new TrackingServiceAdapter(nullptr));
    facade.addService(new CarrierAdapter(nullptr));

    for (const char *path : SCENARIO_FILES) {
        Scenario scenario;
        if (!loadScenario(path, scenario)) {
            std::cerr << "Could not read " << path << " (run tividy from the project root).\n";
            return false;
        }
        scenario.definitionId = facade.registerDefinitionFromFile(scenario.definitionPath);
        if (scenario.definitionId.empty()) {
            std::cerr << "Could not build a workflow from " << scenario.definitionPath << ".\n";
            return false;
        }
        for (const Person &person : scenario.people) {
            facade.addParticipant(person.id, person.name, splitRoles(person.roles));
            people.push_back(person);
        }
        scenarios.push_back(scenario);
    }
    return !people.empty();
}

void Tui::run() {
    std::cout << "\n" << bold("TIVIDY Workflow Engine") << "\n"
              << "One generic engine, " << scenarios.size() << " industries.\n";

    while (!inputClosed) {
        heading("Main menu");
        std::cout << actorLine() << "\n\n"
                  << "  1  Explain the industries\n"
                  << "  2  Start a workflow instance\n"
                  << "  3  Open an instance (" << instances.size() << " started)\n"
                  << "  4  My work\n"
                  << "  5  Change participant\n"
                  << "  6  Undo the last action\n"
                  << "  7  Full audit trail\n"
                  << "  8  Monitoring metrics\n"
                  << "  9  Check deadlines\n"
                  << "  0  Exit\n";

        switch (askNumber("Choose", 0, 9)) {
            case 1: showIndustries(); break;
            case 2: startInstanceScreen(); break;
            case 3: openInstanceScreen(); break;
            case 4: myWorkScreen(); break;
            case 5: chooseActor(); break;
            case 6: undoLast(); break;
            case 7: showAuditTrail(); break;
            case 8: showMetrics(); break;
            case 9: checkDeadlines(); break;
            case 0: std::cout << "Goodbye.\n"; return;
            default: break;
        }
    }
}

// ---- Input ----------------------------------------------------------------------------------

bool Tui::ask(const std::string &prompt, std::string &answer) {
    std::cout << prompt << ": " << std::flush;
    if (!std::getline(std::cin, answer)) {
        inputClosed = true;
        answer.clear();
        std::cout << "\n";
        return false;
    }
    answer = trimmed(answer);
    return true;
}

/// @return The number chosen, or first - 1 if the input was not a number in range.
int Tui::askNumber(const std::string &prompt, int first, int last) {
    std::string answer;
    if (!ask(prompt, answer)) {
        return first - 1;
    }
    char *end = nullptr;
    const long value = std::strtol(answer.c_str(), &end, 10);
    if (answer.empty() || *end != '\0' || value < first || value > last) {
        std::ostringstream message;
        message << "Please enter a number from " << first << " to " << last << ".";
        std::cout << warn(message.str()) << "\n";
        return first - 1;
    }
    return static_cast<int>(value);
}

// ---- Screens --------------------------------------------------------------------------------

void Tui::showIndustries() {
    for (const Scenario &scenario : scenarios) {
        heading(scenario.industry + ": " + scenario.process);
        std::cout << scenario.about << "\n\n"
                  << bold("Expected result: ") << scenario.expected << "\n\n"
                  << bold("Participants") << "\n";
        for (const Person &person : scenario.people) {
            std::cout << "  " << pad(person.name, 22) << person.roles << "\n";
        }
        std::cout << "\n" << bold("Input data") << "\n";
        for (const InputField &input : scenario.inputs) {
            std::cout << "  " << pad(input.key, 16) << input.label << "\n";
        }
        std::cout << "\n" << dim("Definition: " + scenario.definitionPath) << "\n";
    }
    heading("Shared by both industries, unchanged");
    std::cout << "TividyWorkflowFacade, WorkflowEngine, WorkflowInstance, WorkflowDefinition, Task, SubProcess,\n"
              << "WorkItemState and its states, DependencyMediator, RoutingStrategy and its strategies,\n"
              << "ApprovalHandler chain, WorkItemAction commands, WorkflowSnapshot, EventDispatcher and listeners.\n"
              << "Only the data files and the external-service adapters differ.\n";
}

void Tui::startInstanceScreen() {
    heading("Start a workflow instance");
    for (size_t i = 0; i < scenarios.size(); ++i) {
        std::cout << "  " << i + 1 << "  " << pad(scenarios[i].industry, 12) << scenarios[i].process << "\n";
    }
    std::cout << "  0  Back\n";

    const int choice = askNumber("Choose", 0, static_cast<int>(scenarios.size()));
    if (choice <= 0) {
        return;
    }
    const size_t index = static_cast<size_t>(choice - 1);
    const Scenario &scenario = scenarios[index];

    std::cout << "\nEnter the instance data. Press Enter to accept the value in brackets.\n";
    InstanceData data;
    for (const InputField &input : scenario.inputs) {
        std::string value;
        if (!ask(input.label + " [" + input.fallback + "]", value)) {
            return;
        }
        data.set(input.key, value.empty() ? input.fallback : value);
    }

    const std::string id = facade.startInstance(scenario.definitionId, data);
    if (id.empty()) {
        std::cout << bad("The engine could not start this instance.") << "\n";
        return;
    }

    InstanceRecord record;
    record.id = id;
    record.scenario = index;
    instances.push_back(record);

    std::cout << "\n" << good("Started " + id) << "\n";
    flushEvents();
    instanceScreen(record);
}

void Tui::openInstanceScreen() {
    heading("Instances");
    if (instances.empty()) {
        std::cout << "None started yet. Choose 2 on the main menu.\n";
        return;
    }
    for (size_t i = 0; i < instances.size(); ++i) {
        const Scenario &scenario = scenarios[instances[i].scenario];
        std::cout << "  " << pad(std::to_string(i + 1), 3) << pad(instances[i].id, 13)
                  << pad(scenario.industry, 12) << pad(scenario.process, 44)
                  << statusText(facade.getStatus(instances[i].id)) << "\n";
    }
    std::cout << "  0  Back\n";

    const int choice = askNumber("Choose", 0, static_cast<int>(instances.size()));
    if (choice > 0) {
        instanceScreen(instances[static_cast<size_t>(choice - 1)]);
    }
}

void Tui::printWorkTable(const std::vector<WorkItemInfo> &items) {
    std::cout << dim("  " + pad("#", 4) + pad("Work item", 34) + pad("State", 12) + pad("Assignee", 22) + "Outcome") << "\n";
    for (size_t i = 0; i < items.size(); ++i) {
        const WorkItemInfo &item = items[i];
        const std::string name = std::string(static_cast<size_t>(item.depth) * 2, ' ') +
                                 (item.composite ? "[" + item.name + "]" : item.name) +
                                 (item.priority > 0 ? " !" + std::to_string(item.priority) : "");
        std::cout << "  " << pad(std::to_string(i + 1), 4) << pad(name, 34) << stateCell(item.state, 12)
                  << pad(item.assigneeName.empty() ? "-" : item.assigneeName, 22) << shownOutcome(item) << "\n";
    }
}

void Tui::instanceScreen(const InstanceRecord &record) {
    const Scenario &scenario = scenarios[record.scenario];

    while (!inputClosed) {
        const std::vector<WorkItemInfo> items = facade.getWorkItems(record.id);

        heading(record.id + "  " + scenario.industry + ": " + scenario.process);
        std::cout << "Status: " << statusText(facade.getStatus(record.id)) << "    " << actorLine() << "\n\n";
        printWorkTable(items);
        std::cout << "\n" << dim("Number = open that work item   h = history   d = data   p = change participant   u = undo   b = back")
                  << "\n";

        std::string answer;
        if (!ask("Choose", answer)) {
            return;
        }

        if (answer == "b" || answer == "0") return;
        if (answer == "h") { showInstanceHistory(record.id); continue; }
        if (answer == "d") { showInstanceData(record.id); continue; }
        if (answer == "p") { chooseActor(); continue; }
        if (answer == "u") { undoLast(); continue; }

        char *end = nullptr;
        const long number = std::strtol(answer.c_str(), &end, 10);
        if (answer.empty() || *end != '\0' || number < 1 || number > static_cast<long>(items.size())) {
            std::cout << warn("That is not one of the options.") << "\n";
            continue;
        }
        itemScreen(record, items[static_cast<size_t>(number - 1)].id);
    }
}

void Tui::printItem(const WorkItemInfo &item) {
    heading(item.name + " (" + item.id + ")");
    std::cout << "State:     " << stateCell(item.state, 0) << "\n"
              << "Assignee:  " << (item.assigneeName.empty() ? "-" : item.assigneeName) << "\n";
    if (!shownOutcome(item).empty()) {
        std::cout << "Outcome:   " << shownOutcome(item) << "\n";
    }
    if (item.priority > 0) {
        std::cout << "Priority:  " << item.priority << "\n";
    }
    if (item.deadlineHours > 0) {
        std::cout << "Deadline:  " << item.deadlineHours << " hours\n";
    }
    if (item.escalationLevel > 0) {
        std::cout << "Escalated: " << item.escalationLevel << " time(s)\n";
    }
    if (!item.serviceName.empty()) {
        std::cout << "Service:   " << item.serviceName << dim("  (called when work starts)") << "\n";
    }
    if (!item.serviceMessage.empty()) {
        std::cout << "Reply:     " << item.serviceMessage << "\n";
    }
    if (!item.outcomes.empty()) {
        std::cout << "Branches:  ";
        for (size_t i = 0; i < item.outcomes.size(); ++i) {
            std::cout << (i == 0 ? "" : ", ") << item.outcomes[i];
        }
        std::cout << "\n";
    }
}

void Tui::itemScreen(const InstanceRecord &record, const std::string &itemId) {
    WorkItemInfo item;
    if (!findItem(record.id, itemId, item)) {
        return;
    }

    printItem(item);
    std::cout << "\n" << actorLine() << "\n\n"
              << "  1  Claim\n"
              << "  2  Start work\n"
              << "  3  Complete\n"
              << "  4  Reject\n"
              << "  5  Delegate to someone else\n"
              << "  6  Cancel\n";
    const bool canSwitch = !item.assigneeId.empty() && item.assigneeId != people[actor].id;
    if (canSwitch) {
        std::cout << "  7  Act as the assignee (" << item.assigneeName << ")\n";
    }
    std::cout << "  0  Back\n";

    int choice = askNumber("Choose", 0, canSwitch ? 7 : 6);
    if (choice <= 0) {
        return;
    }
    if (choice == 7) {
        actor = personIndex(item.assigneeId);
        std::cout << actorLine() << "\n";
        itemScreen(record, itemId);
        return;
    }
    perform(record, item, choice);
}

void Tui::perform(const InstanceRecord &record, const WorkItemInfo &item, int choice) {
    Participant *who = currentActor();
    const std::string by = " by " + people[actor].name;
    bool accepted = false;
    std::string description;

    switch (choice) {
        case 1:
            accepted = facade.claim(record.id, item.id, who);
            description = "Claim " + item.id + by;
            break;
        case 2:
            accepted = facade.start(record.id, item.id, who);
            description = "Start " + item.id + by;
            break;
        case 3: {
            std::string outcome;
            if (item.outcomes.empty()) {
                if (!ask("Outcome [done]", outcome)) return;
                if (outcome.empty()) outcome = "done";
            } else {
                for (size_t i = 0; i < item.outcomes.size(); ++i) {
                    std::cout << "  " << i + 1 << "  " << item.outcomes[i] << "\n";
                }
                const int picked = askNumber("Outcome", 1, static_cast<int>(item.outcomes.size()));
                if (picked < 1) return;
                outcome = item.outcomes[static_cast<size_t>(picked - 1)];
            }
            accepted = facade.complete(record.id, item.id, who, outcome);
            description = "Complete " + item.id + " (" + outcome + ")" + by;
            break;
        }
        case 4: {
            std::string reason;
            if (!ask("Reason for rejecting", reason)) return;
            if (reason.empty()) {
                std::cout << warn("A rejection needs a reason.") << "\n";
                return;
            }
            accepted = facade.reject(record.id, item.id, who, reason);
            description = "Reject " + item.id + " (" + reason + ")" + by;
            break;
        }
        case 5: {
            printRoster();
            const int picked = askNumber("Delegate to", 1, static_cast<int>(people.size()));
            if (picked < 1) return;
            const Person &target = people[static_cast<size_t>(picked - 1)];
            accepted = facade.delegate(record.id, item.id, who, facade.findParticipant(target.id));
            description = "Delegate " + item.id + " to " + target.name + by;
            break;
        }
        case 6:
            accepted = facade.cancel(record.id, item.id, who);
            description = "Cancel " + item.id + by;
            break;
        default:
            return;
    }

    if (!accepted) {
        std::cout << "\n" << bad("Refused by the engine: ") << describeRefusal(item, choice) << "\n";
        return;
    }

    UndoEntry entry;
    entry.instanceId = record.id;
    entry.description = description;
    undoStack.push_back(entry);

    std::cout << "\n" << good("Done: ") << description << "\n";
    flushEvents();

    // Starting work may have called an external service; show what it said.
    WorkItemInfo after;
    if (choice == 2 && findItem(record.id, item.id, after) && !after.serviceMessage.empty()) {
        const bool failed = after.state == "Escalated";
        std::cout << bold(after.serviceName + " replied: ") << (failed ? bad(after.serviceMessage) : after.serviceMessage) << "\n";
        if (failed) {
            std::cout << warn("The call failed, so the item escalated. Claim it to try again, or cancel it.") << "\n";
        } else {
            std::cout << dim("The reply is in the instance data (d). Complete the item with the outcome it calls for.") << "\n";
        }
    }
}

std::string Tui::describeRefusal(const WorkItemInfo &item, int choice) const {
    const std::string assignee = item.assigneeName.empty() ? "nobody" : item.assigneeName;
    const bool mine = item.assigneeId == people[actor].id;

    if (choice == 1 && !item.assigneeId.empty()) {
        return "it is already assigned to " + assignee + "; they can delegate it instead.";
    }
    if (choice >= 2 && choice <= 5 && item.assigneeId.empty()) {
        return "it has no assignee yet (it is " + item.state + ").";
    }
    if (choice >= 2 && choice <= 5 && !mine) {
        return "only the assignee may do this, and it is assigned to " + assignee + ".";
    }
    return "this action is not legal while the item is " + item.state + ".";
}

void Tui::myWorkScreen() {
    struct Row {
        const InstanceRecord *record;
        WorkItemInfo item;
    };

    std::vector<Row> rows;
    for (const InstanceRecord &record : instances) {
        for (const WorkItemInfo &item : facade.getWorkItems(record.id)) {
            const bool open = item.state != "Completed" && item.state != "Rejected" && item.state != "Cancelled";
            const bool unclaimed = item.assigneeId.empty() && (item.state == "Available" || item.state == "Escalated");
            if (open && !item.composite && (item.assigneeId == people[actor].id || unclaimed)) {
                Row row;
                row.record = &record;
                row.item = item;
                rows.push_back(row);
            }
        }
    }

    // Most urgent first; PriorityDecorator sets the level.
    std::stable_sort(rows.begin(), rows.end(), [](const Row &a, const Row &b) { return a.item.priority > b.item.priority; });

    heading("My work");
    std::cout << actorLine() << "\n\n";
    if (rows.empty()) {
        std::cout << "Nothing assigned to you, and nothing waiting to be claimed.\n";
        return;
    }
    std::cout << dim("  " + pad("#", 4) + pad("Instance", 13) + pad("Work item", 34) + pad("State", 12) + "Assigned to") << "\n";
    for (size_t i = 0; i < rows.size(); ++i) {
        std::cout << "  " << pad(std::to_string(i + 1), 4) << pad(rows[i].record->id, 13) << pad(rows[i].item.name + (rows[i].item.priority > 0 ? " !" + std::to_string(rows[i].item.priority) : ""), 34)
                  << stateCell(rows[i].item.state, 12)
                  << (rows[i].item.assigneeId.empty() ? warn("waiting to be claimed") : "you") << "\n";
    }
    std::cout << "  0  Back\n";

    const int choice = askNumber("Choose", 0, static_cast<int>(rows.size()));
    if (choice > 0) {
        const Row &row = rows[static_cast<size_t>(choice - 1)];
        itemScreen(*row.record, row.item.id);
    }
}

void Tui::printRoster() {
    for (size_t i = 0; i < people.size(); ++i) {
        std::cout << "  " << pad(std::to_string(i + 1), 3) << pad(people[i].industry, 11)
                  << pad(people[i].name, 22) << people[i].roles << "\n";
    }
}

void Tui::chooseActor() {
    heading("Change participant");
    printRoster();
    const int choice = askNumber("Act as", 1, static_cast<int>(people.size()));
    if (choice >= 1) {
        actor = static_cast<size_t>(choice - 1);
        std::cout << actorLine() << "\n";
    }
}

void Tui::undoLast() {
    if (undoStack.empty()) {
        std::cout << warn("There is nothing to undo.") << "\n";
        return;
    }

    const UndoEntry entry = undoStack.back();
    undoStack.pop_back();

    if (!facade.undoLast()) {
        std::cout << bad("The engine could not undo that action.") << "\n";
        return;
    }

    HistoryEntry row;
    row.instanceId = entry.instanceId;
    row.when = std::time(nullptr);
    row.what = "UNDONE";
    row.detail = entry.description;
    history.push_back(row);

    std::cout << good("Undone in " + entry.instanceId + ": ") << entry.description << "\n"
              << dim("The instance was restored from the snapshot taken before that action. External service calls cannot be recalled.") << "\n";
}

void Tui::showInstanceHistory(const std::string &instanceId) {
    heading("History of " + instanceId);
    for (const HistoryEntry &row : history) {
        if (row.instanceId == instanceId) {
            std::cout << "  " << dim(clock(row.when)) << "  " << pad(row.what, 11) << pad(row.itemName, 30) << row.detail << "\n";
        }
    }
}

void Tui::showInstanceData(const std::string &instanceId) {
    heading("Data of " + instanceId);
    const std::map<std::string, std::string> data = facade.getInstanceData(instanceId);
    for (std::map<std::string, std::string>::const_iterator it = data.begin(); it != data.end(); ++it) {
        std::cout << "  " << pad(it->first, 20) << it->second << "\n";
    }
}

void Tui::showAuditTrail() {
    heading("Audit trail (AuditLogger, all instances)");
    if (audit->getEntryCount() == 0) {
        std::cout << "Nothing has happened yet.\n";
        return;
    }
    audit->printAuditTrail();

    const std::vector<std::string> &attempts = facade.getAuditedAttempts();
    if (!attempts.empty()) {
        heading("Attempts on audited items (AuditDecorator, including refused ones)");
        for (const std::string &attempt : attempts) {
            std::cout << attempt << "\n";
        }
    }
}

void Tui::checkDeadlines() {
    heading("Check deadlines");
    std::cout << "Items with a deadline escalate once it passes. Deadlines run in hours, so choose how far ahead to check.\n";

    std::string answer;
    if (!ask("Hours from now [0]", answer)) {
        return;
    }
    char *end = nullptr;
    const long hours = answer.empty() ? 0 : std::strtol(answer.c_str(), &end, 10);
    if (!answer.empty() && (*end != '\0' || hours < 0 || hours > 100000)) {
        std::cout << warn("Please enter a whole number of hours.") << "\n";
        return;
    }

    facade.checkDeadlines(std::time(nullptr) + hours * 3600);

    if (events->empty()) {
        std::cout << "No deadlines have passed at that time.\n";
        return;
    }
    flushEvents();
}

void Tui::showMetrics() {
    heading("Monitoring metrics (MonitoringService)");

    int running = 0;
    int completed = 0;
    int terminated = 0;
    for (const InstanceRecord &record : instances) {
        const std::string status = facade.getStatus(record.id);
        if (status == "completed") ++completed;
        else if (status == "terminated") ++terminated;
        else ++running;
    }
    std::cout << "  Instances   " << running << " running, " << completed << " completed, " << terminated << " terminated\n"
              << "  Events      " << monitor->getTotalEvents() << "\n"
              << "  Failures    " << monitor->getFailureRate() << "% of events\n"
              << "  Notified    " << notifier->getNotificationCount() << " messages\n\n";

    const EventType types[] = {EventType::AVAILABLE, EventType::ASSIGNED, EventType::STARTED, EventType::COMPLETED,
                               EventType::REJECTED, EventType::CANCELLED, EventType::ESCALATED};
    for (EventType type : types) {
        std::cout << "  " << pad(eventTypeToString(type), 12) << monitor->getCountForType(type) << "\n";
    }
}

// ---- Helpers --------------------------------------------------------------------------------

/// Shows what the last action caused, and keeps it as history.
void Tui::flushEvents() {
    for (const WorkflowEvent &event : events->take()) {
        const bool hasOutcome = event.type == EventType::COMPLETED || event.type == EventType::REJECTED ||
                                event.type == EventType::CANCELLED || event.type == EventType::ESCALATED;
        std::string detail = hasOutcome ? event.outcome : "";

        WorkItemInfo item;
        const bool found = findItem(event.instanceId, event.itemId, item);
        if (event.type == EventType::ASSIGNED && found) {
            detail = "assigned to " + item.assigneeName;
        }
        if (event.type == EventType::ESCALATED && detail.empty()) {
            detail = "no participant holds the role this work needs";
        }

        HistoryEntry row;
        row.instanceId = event.instanceId;
        row.when = event.timestamp;
        row.itemName = event.itemName;
        row.what = eventTypeToString(event.type);
        row.detail = detail;
        history.push_back(row);

        std::cout << "  " << dim("event") << "  " << pad(row.what, 11) << pad(row.itemName, 30) << detail << "\n";
    }
}

bool Tui::findItem(const std::string &instanceId, const std::string &itemId, WorkItemInfo &item) {
    for (const WorkItemInfo &candidate : facade.getWorkItems(instanceId)) {
        if (candidate.id == itemId) {
            item = candidate;
            return true;
        }
    }
    return false;
}

size_t Tui::personIndex(const std::string &id) const {
    for (size_t i = 0; i < people.size(); ++i) {
        if (people[i].id == id) {
            return i;
        }
    }
    return actor;
}

Participant *Tui::currentActor() {
    return facade.findParticipant(people[actor].id);
}

std::string Tui::actorLine() const {
    return "Acting as: " + bold(people[actor].name) + dim(" (" + people[actor].roles + ")");
}

} // namespace

int main() {
    Tui tui;
    if (!tui.initialise()) {
        return 1;
    }
    tui.run();
    return 0;
}
