/**
 * @file main.cpp
 * @brief Main interactive Terminal User Interface (TUI) entry point for the TIVIDY Workflow Engine.
 * @details The TUI drives the engine through TividyWorkflowFacade. Every workflow in definitions/
 *          is built, and a workflow can be run once a file in scenarios/ names it by id and lists
 *          its participants and the details they enter, so nothing below knows about any
 *          particular workflow, and instance data is only ever typed in by a participant.
 *          Every action stays on offer even when it is illegal: the engine decides, and the TUI
 *          reports why it refused.
 */

#include "ApprovalRouting.h"
#include "AuditLogger.h"
#include "BankGatewayAdapter.h"
#include "CarrierAdapter.h"
#include "DataRuleRouting.h"
#include "GovernmentIDAdapter.h"
#include "MonitoringService.h"
#include "NotificationService.h"
#include "Participant.h"
#include "PriorityDecorator.h"
#include "RoleBasedRouting.h"
#include "Task.h"
#include "TividyWorkflowFacade.h"
#include "TrackingServiceAdapter.h"
#include "WorkItem.h"
#include "WorkItemDecorator.h"
#include "WorkflowEventListener.h"
#include "WorkflowInstance.h"

#include <dirent.h>
#include <sys/ioctl.h>
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

const char *const DEFINITIONS_FOLDER = "definitions";
const char *const SCENARIOS_FOLDER = "scenarios";

// 256-colour palette
const int GOLD = 220;
const int SKY = 81;
const int BLUE = 39;
const int VIOLET = 141;
const int PINK = 213;
const int GREEN = 84;
const int RED = 203;
const int ORANGE = 214;
const int GREY = 245;
const int WHITE = 255;
const int NAVY = 17;
const int BLACK = 16;

/// A participant as listed in a scenario file.
struct Person {
    std::string id;       // unique within its scenario
    std::string name;
    std::string roles;
    size_t scenario;      // index into scenarios
};

/// A detail a participant types in. The scenario names what to ask; the values are never stored in a file.
struct FieldPrompt {
    std::string key;
    std::string label;
};

/// One runnable workflow: what to tell the user, and what to give the engine.
struct Scenario {
    std::string industry;
    std::string workflowId;
    std::string roster;      // the engine roster holding this scenario's participants
    std::string process;     // from the definition
    std::string about;       // from the definition
    std::string expected;
    std::vector<Person> people;
    std::string starterRole;                                 // empty: any participant may start it
    std::vector<FieldPrompt> startFields;                    // asked when an instance is started
    std::map<std::string, std::vector<FieldPrompt>> taskFields; // asked when that task is completed
    std::map<std::string, std::vector<std::string>> outcomes;
};

/// A running instance and the scenario it was started from.
struct InstanceRecord {
    std::string id;
    std::string summary;   // who started it and what they entered
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

/// One work item as shown in a table, with its position in the tree drawn.
struct Row {
    WorkItem *item;
    std::string branch;
};

// ---- Terminal -------------------------------------------------------------------------------

bool outputIsTerminal() {
    static const bool terminal = isatty(STDOUT_FILENO) != 0;
    return terminal;
}

bool inputIsTerminal() {
    static const bool terminal = isatty(STDIN_FILENO) != 0;
    return terminal;
}

bool useColour() {
    static const bool enabled = outputIsTerminal() && std::getenv("NO_COLOR") == nullptr;
    return enabled;
}

size_t screenWidth() {
    struct winsize size;
    if (outputIsTerminal() && ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0 && size.ws_col > 0) {
        return std::min<size_t>(std::max<size_t>(size.ws_col, 84), 120);
    }
    return 100;
}

std::string fg(int colour, const std::string &text) {
    return useColour() ? "\033[38;5;" + std::to_string(colour) + "m" + text + "\033[0m" : text;
}

std::string bold(int colour, const std::string &text) {
    return useColour() ? "\033[1;38;5;" + std::to_string(colour) + "m" + text + "\033[0m" : text;
}

/// Coloured label. Without colour it falls back to brackets so it still stands out.
std::string pill(int background, int foreground, const std::string &text) {
    if (!useColour()) {
        return "[" + text + "]";
    }
    return "\033[1;48;5;" + std::to_string(background) + ";38;5;" + std::to_string(foreground) + "m " + text + " \033[0m";
}

/// Columns a string fills on screen, ignoring colour codes and counting UTF-8 characters once.
size_t visibleWidth(const std::string &text) {
    size_t width = 0;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\033') {
            while (i < text.size() && text[i] != 'm') ++i;
            continue;
        }
        if ((static_cast<unsigned char>(text[i]) & 0xC0) != 0x80) ++width;
    }
    return width;
}

std::string pad(const std::string &text, size_t width) {
    const size_t used = visibleWidth(text);
    return used >= width ? text : text + std::string(width - used, ' ');
}

/// Shortens plain ASCII text to fit a column.
std::string fit(const std::string &text, size_t width) {
    if (text.size() <= width) return text;
    return width <= 3 ? text.substr(0, width) : text.substr(0, width - 3) + "...";
}

std::string repeat(const std::string &piece, size_t count) {
    std::string result;
    for (size_t i = 0; i < count; ++i) result += piece;
    return result;
}

std::vector<std::string> wrap(const std::string &text, size_t width) {
    std::vector<std::string> lines;
    std::istringstream words(text);
    std::string word;
    std::string line;
    while (words >> word) {
        if (!line.empty() && line.size() + 1 + word.size() > width) {
            lines.push_back(line);
            line.clear();
        }
        line += (line.empty() ? "" : " ") + word;
    }
    if (!line.empty()) lines.push_back(line);
    return lines;
}

void boxTop(const std::string &title, int colour) {
    const size_t width = screenWidth();
    const std::string label = title.empty() ? "" : " " + title + " ";
    std::cout << fg(colour, "╭─") << bold(colour, label)
              << fg(colour, repeat("─", width - 3 - visibleWidth(label)) + "╮") << "\n";
}

void boxLine(const std::string &content, int colour) {
    std::cout << fg(colour, "│") << " " << pad(content, screenWidth() - 4) << " " << fg(colour, "│") << "\n";
}

/// Splits a definition's description at its line breaks, keeping the empty lines between paragraphs.
std::vector<std::string> descriptionLines(const std::string &description) {
    std::vector<std::string> lines;
    std::istringstream in(description);
    std::string line;
    while (std::getline(in, line)) {
        lines.push_back(line);
    }
    return lines;
}

void boxBottom(int colour) {
    std::cout << fg(colour, "╰" + repeat("─", screenWidth() - 2) + "╯") << "\n";
}

void boxText(const std::string &text, int colour) {
    for (const std::string &line : wrap(text, screenWidth() - 4)) {
        boxLine(line, colour);
    }
}

std::string progressBar(int done, int total, size_t cells) {
    const size_t filled = total == 0 ? 0 : static_cast<size_t>(done) * cells / static_cast<size_t>(total);
    return bold(GREEN, repeat("█", filled)) + fg(238, repeat("░", cells - filled));
}

std::string stateIcon(const std::string &state) {
    if (state == "Completed") return "✓";
    if (state == "Rejected") return "✗";
    if (state == "Cancelled") return "-";
    if (state == "Escalated") return "▲";
    if (state == "InProgress") return "▶";
    if (state == "Assigned") return "●";
    if (state == "Available") return "◆";
    return "○";
}

int stateColour(const std::string &state) {
    if (state == "Completed") return GREEN;
    if (state == "Rejected") return RED;
    if (state == "Cancelled") return GREY;
    if (state == "Escalated") return ORANGE;
    if (state == "InProgress") return BLUE;
    if (state == "Assigned") return SKY;
    if (state == "Available") return GOLD;
    return 240;
}

std::string stateCell(const std::string &state, size_t width) {
    return pad(bold(stateColour(state), stateIcon(state) + " " + state), width);
}

std::string statusPill(const std::string &status) {
    if (status == "completed") return pill(GREEN, BLACK, "COMPLETED");
    if (status == "terminated") return pill(RED, BLACK, "TERMINATED");
    return pill(BLUE, BLACK, "RUNNING");
}

int eventColour(EventType type) {
    switch (type) {
        case EventType::COMPLETED: return GREEN;
        case EventType::REJECTED: return RED;
        case EventType::CANCELLED: return GREY;
        case EventType::ESCALATED: return ORANGE;
        case EventType::FAILED: return RED;
        case EventType::STARTED: return BLUE;
        case EventType::ASSIGNED: return SKY;
        case EventType::AVAILABLE: return GOLD;
        default: return VIOLET;
    }
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

std::vector<std::string> split(const std::string &text, char separator) {
    std::vector<std::string> parts;
    std::istringstream in(text);
    std::string part;
    while (std::getline(in, part, separator)) {
        parts.push_back(trimmed(part));
    }
    return parts;
}

/// Reads a scenario file; anything it cannot use is added to problems so that a typo is easy to find.
bool loadScenario(const std::string &path, Scenario &scenario, std::vector<std::string> &problems) {
    std::ifstream file(path.c_str());
    if (!file) {
        problems.push_back(path + ": cannot be read");
        return false;
    }

    std::string line;
    size_t number = 0;
    while (std::getline(file, line)) {
        ++number;
        line = trimmed(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }
        const std::vector<std::string> fields = split(line, '|');
        const std::string &key = fields[0];
        if (fields.size() == 2 && key == "INDUSTRY") scenario.industry = fields[1];
        else if (fields.size() == 2 && key == "WORKFLOW") scenario.workflowId = fields[1];
        else if (fields.size() == 2 && key == "EXPECTED") scenario.expected = fields[1];
        else if (fields.size() == 2 && key == "STARTED_BY") scenario.starterRole = fields[1];
        else if (fields.size() == 3 && key == "OUTCOMES") scenario.outcomes[fields[1]] = split(fields[2], ',');
        else if (fields.size() == 3 && key == "FIELD") scenario.startFields.push_back(FieldPrompt{fields[1], fields[2]});
        else if (fields.size() == 4 && key == "TASK_FIELD") scenario.taskFields[fields[1]].push_back(FieldPrompt{fields[2], fields[3]});
        else if (fields.size() == 4 && key == "PARTICIPANT") {
            Person person;
            person.id = fields[1];
            person.name = fields[2];
            person.roles = fields[3];
            person.scenario = 0;
            scenario.people.push_back(person);
        } else {
            problems.push_back(path + " line " + std::to_string(number) + " is not understood: " + line);
        }
    }

    if (scenario.workflowId.empty()) {
        problems.push_back(path + ": needs a WORKFLOW|<id> line");
        return false;
    }
    if (scenario.people.empty()) {
        problems.push_back(path + ": needs at least one PARTICIPANT line");
        return false;
    }
    return true;
}

bool folderExists(const std::string &folder) {
    DIR *directory = opendir(folder.c_str());
    if (directory == nullptr) {
        return false;
    }
    closedir(directory);
    return true;
}

/// "scenarios/education.scenario" becomes "education".
std::string fileStem(const std::string &path) {
    const size_t slash = path.find_last_of('/');
    const std::string name = slash == std::string::npos ? path : path.substr(slash + 1);
    return name.substr(0, name.find_last_of('.'));
}

/// Paths of the files in a folder with this extension, sorted so menus keep a stable order.
std::vector<std::string> filesIn(const std::string &folder, const std::string &extension) {
    std::vector<std::string> paths;
    DIR *directory = opendir(folder.c_str());
    if (directory == nullptr) {
        return paths;
    }
    while (const dirent *entry = readdir(directory)) {
        const std::string name = entry->d_name;
        if (name.size() > extension.size() && name.compare(name.size() - extension.size(), extension.size(), extension) == 0) {
            paths.push_back(folder + "/" + name);
        }
    }
    closedir(directory);
    std::sort(paths.begin(), paths.end());
    return paths;
}

// ---- Work items -----------------------------------------------------------------------------

/// The task behind any decorators, or nullptr for a sub-process.
Task *taskOf(WorkItem *item) {
    while (WorkItemDecorator *decorator = dynamic_cast<WorkItemDecorator *>(item)) {
        item = decorator->getInner();
    }
    return dynamic_cast<Task *>(item);
}

/// Priority set by a PriorityDecorator anywhere around the item; 0 if there is none.
int priorityOf(WorkItem *item) {
    while (WorkItemDecorator *decorator = dynamic_cast<WorkItemDecorator *>(item)) {
        if (PriorityDecorator *priority = dynamic_cast<PriorityDecorator *>(decorator)) {
            return priority->getPriority();
        }
        item = decorator->getInner();
    }
    return 0;
}

/// The role the item's routing strategy asks for, worked out from this instance's data.
std::string requiredRole(Task *task, const InstanceData &data) {
    if (task == nullptr) {
        return "";
    }
    if (const RoleBasedRouting *role = dynamic_cast<const RoleBasedRouting *>(task->getRouting())) {
        return role->getRole();
    }
    if (const DataRuleRouting *rule = dynamic_cast<const DataRuleRouting *>(task->getRouting())) {
        const std::map<std::string, std::string> mapping = rule->getMapping();
        const std::map<std::string, std::string>::const_iterator found = mapping.find(data.get(rule->getField()));
        return found != mapping.end() ? found->second : rule->getFallbackRole();
    }
    return "";
}

bool isFinal(const std::string &state) {
    return state == "Completed" || state == "Rejected" || state == "Cancelled";
}

/// An outcome only means something once work has finished or escalated; a retry may leave an old one behind.
std::string shownOutcome(WorkItem *item) {
    const std::string state = item->getStateName();
    return isFinal(state) || state == "Escalated" ? item->getOutcome() : "";
}

void collectRows(WorkItem *item, const std::string &indent, bool last, bool root, std::vector<Row> &rows) {
    Row row;
    row.item = item;
    row.branch = root ? "" : indent + (last ? "└─ " : "├─ ");
    rows.push_back(row);

    const std::string childIndent = root ? "" : indent + (last ? "   " : "│  ");
    for (int i = 0; i < item->childCount(); ++i) {
        collectRows(item->getChild(i), childIndent, i == item->childCount() - 1, false, rows);
    }
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
    EventQueue *events = nullptr;            // owned by the engine
    AuditLogger *audit = nullptr;            // owned by the engine
    MonitoringService *monitor = nullptr;    // owned by the engine
    NotificationService *notifier = nullptr; // owned by the engine

    std::vector<Scenario> scenarios;
    std::vector<Person> people;
    std::vector<InstanceRecord> instances;
    std::vector<HistoryEntry> history;
    std::vector<std::string> notices;
    std::vector<std::string> notReady;     // why a file in definitions/ or scenarios/ cannot run yet
    size_t current = 0;    // scenario the user chose on the entry screen
    size_t actor = 0;      // index into people, chosen after the scenario
    bool inputClosed = false;
    bool checkingDeadlines = false;   // explains the escalations a deadline check causes

    // Input
    bool ask(const std::string &prompt, std::string &answer);
    int askNumber(const std::string &prompt, int first, int last);
    void pause();

    // Screens
    void beginScreen(const std::string &title);
    void mainMenu();
    void showIndustry();
    void startInstanceScreen();
    void openInstanceScreen();
    void instanceScreen(const InstanceRecord &record);
    void itemScreen(const InstanceRecord &record, const std::string &itemId);
    void myWorkScreen();
    bool chooseActor();
    void undoLast();
    void showInstanceHistory(const std::string &instanceId);
    void showInstanceData(const std::string &instanceId);
    void showCommands(const std::string &instanceId);
    void showAuditTrail();
    void showMetrics();
    void checkDeadlines();

    // Helpers
    std::vector<size_t> printRoster();
    std::vector<const InstanceRecord *> currentInstances() const;
    void flushEvents();
    void perform(const InstanceRecord &record, WorkItem *item, int choice);
    void noticeOk(const std::string &text);
    void noticeWarning(const std::string &text);
    void noticeError(const std::string &text);
    size_t personIndex(const std::string &id) const;
    Participant *currentActor() const;
    std::string actorLabel() const;
    void taskProgress(const std::string &instanceId, int &done, int &total) const;
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
    facade.addService(new GovernmentIDAdapter());
    facade.addService(new BankGatewayAdapter());
    facade.addService(new TrackingServiceAdapter());
    facade.addService(new CarrierAdapter());

    if (!folderExists(DEFINITIONS_FOLDER) || !folderExists(SCENARIOS_FOLDER)) {
        std::cerr << "Run tividy from the project root, which holds " << DEFINITIONS_FOLDER << "/ and "
                  << SCENARIOS_FOLDER << "/.\n";
        return false;
    }

    // Every definition is built. A bad file only costs that workflow, not the whole program.
    for (const std::string &path : filesIn(DEFINITIONS_FOLDER, ".txt")) {
        if (facade.registerDefinitionFromFile(path).empty()) {
            // Builder errors already start with the path; others don't.
            const std::string error = facade.getLastError();
            notReady.push_back(error.compare(0, path.size(), path) == 0 ? error : path + ": " + error);
        }
    }

    // A scenario makes its workflow runnable, by naming the id in the definition's DEFINITION section.
    std::vector<std::string> covered;
    for (const std::string &path : filesIn(SCENARIOS_FOLDER, ".scenario")) {
        Scenario scenario;
        if (!loadScenario(path, scenario, notReady)) {
            continue;
        }
        scenario.process = facade.definitionName(scenario.workflowId);
        scenario.about = facade.definitionDescription(scenario.workflowId);
        if (scenario.process.empty()) {
            notReady.push_back(path + ": no definition has the id " + scenario.workflowId);
            continue;
        }
        if (scenario.industry.empty()) {
            scenario.industry = scenario.process;
        }
        // Each scenario owns its participants, so names, ids and roles can repeat across scenarios.
        scenario.roster = fileStem(path);
        for (Person &person : scenario.people) {
            person.scenario = scenarios.size();
            facade.addParticipant(person.id, person.name, split(person.roles, ','), scenario.roster);
            if (!facade.getLastError().empty()) {
                notReady.push_back(path + ": participant " + person.id + " is listed twice");
                continue;
            }
            people.push_back(person);
        }
        covered.push_back(scenario.workflowId);
        scenarios.push_back(scenario);
    }

    for (const std::string &id : facade.definitionIds()) {
        if (std::find(covered.begin(), covered.end(), id) == covered.end()) {
            notReady.push_back("Workflow " + id + " has no scenario yet: add one with WORKFLOW|" + id);
        }
    }
    return true;
}

void Tui::run() {
    while (!inputClosed) {
        if (outputIsTerminal()) std::cout << "\033[2J\033[H";

        const int shades[] = {51, 45, 39, 33, 63, 99};
        const char *const banner[] = {
            "████████╗██╗██╗   ██╗██╗██████╗ ██╗   ██╗",
            "╚══██╔══╝██║██║   ██║██║██╔══██╗╚██╗ ██╔╝",
            "   ██║   ██║██║   ██║██║██║  ██║ ╚████╔╝ ",
            "   ██║   ██║╚██╗ ██╔╝██║██║  ██║  ╚██╔╝  ",
            "   ██║   ██║ ╚████╔╝ ██║██████╔╝   ██║   ",
            "   ╚═╝   ╚═╝  ╚═══╝  ╚═╝╚═════╝    ╚═╝   ",
        };
        std::cout << "\n";
        for (int i = 0; i < 6; ++i) {
            std::cout << "  " << bold(shades[i], banner[i]) << "\n";
        }
        std::cout << "  " << fg(GREY, "One workflow engine. ") << bold(GOLD, std::to_string(scenarios.size()) + " ready to run.") << "\n\n";

        if (!notices.empty()) {
            boxTop("What just happened", GOLD);
            for (const std::string &notice : notices) boxLine(notice, GOLD);
            boxBottom(GOLD);
            notices.clear();
        }

        if (!notReady.empty()) {
            boxTop("Not ready", ORANGE);
            for (const std::string &problem : notReady) {
                const std::vector<std::string> lines = wrap(problem, screenWidth() - 6);
                for (size_t i = 0; i < lines.size(); ++i) {
                    boxLine((i == 0 ? bold(ORANGE, "▲ ") : std::string("  ")) + lines[i], ORANGE);
                }
            }
            boxBottom(ORANGE);
        }

        boxTop("Where are we going?", BLUE);
        if (scenarios.empty()) {
            boxLine(fg(GREY, "Nothing can run yet. Add a workflow to definitions/ and a scenario naming it to scenarios/."), BLUE);
        }
        size_t nameWidth = 0;
        for (const Scenario &scenario : scenarios) nameWidth = std::max(nameWidth, scenario.industry.size());
        for (size_t i = 0; i < scenarios.size(); ++i) {
            boxLine(bold(GREEN, pad(" " + std::to_string(i + 1), 4)) + pad(bold(WHITE, scenarios[i].industry), nameWidth + 3) + fg(GREY, scenarios[i].process), BLUE);
        }
        boxLine(bold(RED, " 0 ") + " " + bold(WHITE, "Exit"), BLUE);
        boxBottom(BLUE);

        const int choice = askNumber("Choose", 0, static_cast<int>(scenarios.size()));
        if (choice == 0) {
            std::cout << bold(SKY, "Goodbye.") << "\n";
            return;
        }
        if (choice > 0) {
            current = static_cast<size_t>(choice - 1);
            if (chooseActor()) {
                mainMenu();
            }
        }
    }
}

void Tui::mainMenu() {
    while (!inputClosed) {
        beginScreen(scenarios[current].industry + "  ›  " + scenarios[current].process);

        const std::vector<const InstanceRecord *> mine = currentInstances();
        int running = 0;
        for (const InstanceRecord *record : mine) {
            if (facade.getStatus(record->id) == "running") ++running;
        }

        const struct { int key; int colour; const char *name; const char *hint; } options[] = {
            {1, VIOLET, "Explain this workflow", "what the process does and who is involved"},
            {2, GREEN, "Start a workflow instance", "enter the details and run it"},
            {3, SKY, "Open an instance", ""},
            {4, GOLD, "My work", "what you can act on right now"},
            {5, PINK, "Change participant", "choose who you are acting as"},
            {6, ORANGE, "Undo the last action", "roll back from a snapshot"},
            {7, BLUE, "Full audit trail", "every event, all workflows"},
            {8, BLUE, "Monitoring metrics", "live counts from the observers"},
            {9, ORANGE, "Check deadlines", "jump ahead and see what escalates"},
        };

        boxTop("Main menu", BLUE);
        boxLine("Acting as " + actorLabel(), BLUE);
        boxLine("", BLUE);
        for (const auto &option : options) {
            std::string hint = option.hint;
            if (option.key == 3) {
                hint = std::to_string(mine.size()) + " started, " + std::to_string(running) + " running";
            }
            boxLine(bold(option.colour, " " + std::to_string(option.key) + " ") + " " + pad(bold(WHITE, option.name), 28) + fg(GREY, hint), BLUE);
        }
        boxLine(bold(RED, " 0 ") + " " + bold(WHITE, "Back to the workflow list"), BLUE);
        boxBottom(BLUE);

        switch (askNumber("Choose", 0, 9)) {
            case 1: showIndustry(); break;
            case 2: startInstanceScreen(); break;
            case 3: openInstanceScreen(); break;
            case 4: myWorkScreen(); break;
            case 5: chooseActor(); break;
            case 6: undoLast(); break;
            case 7: showAuditTrail(); break;
            case 8: showMetrics(); break;
            case 9: checkDeadlines(); break;
            case 0: return;
            default: break;
        }
    }
}

// ---- Input ----------------------------------------------------------------------------------

bool Tui::ask(const std::string &prompt, std::string &answer) {
    std::cout << bold(PINK, "❯ ") << bold(WHITE, prompt) << ": " << std::flush;
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
        noticeWarning("Please enter a number from " + std::to_string(first) + " to " + std::to_string(last) + ".");
        return first - 1;
    }
    return static_cast<int>(value);
}

/// Keeps a read-only screen up until the user is done with it; scripted input is never consumed.
void Tui::pause() {
    if (!inputIsTerminal() || inputClosed) return;
    std::string ignored;
    ask("Press Enter to go back", ignored);
}

// ---- Screens --------------------------------------------------------------------------------

void Tui::beginScreen(const std::string &title) {
    if (outputIsTerminal()) std::cout << "\033[2J\033[H";

    const size_t width = screenWidth();
    const std::string left = "  TIVIDY  ›  " + title;
    if (useColour()) {
        std::cout << "\033[1;48;5;" << NAVY << ";38;5;" << WHITE << "m" << pad(left, width) << "\033[0m\n\n";
    } else {
        std::cout << left << "\n" << repeat("=", width) << "\n\n";
    }

    if (!notices.empty()) {
        boxTop("What just happened", GOLD);
        for (const std::string &notice : notices) boxLine(notice, GOLD);
        boxBottom(GOLD);
        std::cout << "\n";
        notices.clear();
    }
}

void Tui::showIndustry() {
    const Scenario &scenario = scenarios[current];
    beginScreen(scenario.industry + "  ›  About");

    const int colour = VIOLET;
    boxTop(scenario.process, colour);
    for (const std::string &paragraph : descriptionLines(scenario.about)) {
        if (paragraph.empty()) boxLine("", colour);
        else boxText(paragraph, colour);
    }
    boxLine("", colour);
    if (!scenario.expected.empty()) {
        boxLine(bold(colour, "Expected result"), colour);
        boxText(scenario.expected, colour);
        boxLine("", colour);
    }
    boxLine(bold(colour, "Participants"), colour);
    for (const Person &person : scenario.people) {
        boxLine("  " + pad(bold(WHITE, person.name), 22) + fg(GREY, person.roles), colour);
    }
    boxLine("", colour);
    if (!scenario.starterRole.empty()) {
        boxLine(bold(colour, "Started by"), colour);
        boxLine("  " + scenario.starterRole, colour);
        boxLine("", colour);
    }
    if (!scenario.startFields.empty()) boxLine(bold(colour, "Details asked for at the start"), colour);
    for (const FieldPrompt &field : scenario.startFields) {
        boxLine("  " + fit(field.label, screenWidth() - 8), colour);
    }
    for (const std::pair<const std::string, std::vector<FieldPrompt>> &task : scenario.taskFields) {
        boxLine(bold(colour, "Details asked for at " + task.first), colour);
        for (const FieldPrompt &field : task.second) {
            boxLine("  " + fit(field.label, screenWidth() - 8), colour);
        }
    }
    boxLine("", colour);
    boxLine(fg(GREY, "Workflow: " + scenario.workflowId + "  (from " + DEFINITIONS_FOLDER + "/)"), colour);
    boxBottom(colour);
    std::cout << "\n";

    boxTop("Shared by every workflow, unchanged", BLUE);
    boxText("TividyWorkflowFacade, WorkflowEngine, WorkflowInstance, WorkflowDefinition, Task, SubProcess, "
            "WorkItemState and its states, DependencyMediator, RoutingStrategy and its strategies, the "
            "ApprovalHandler chain, WorkItemAction commands, WorkflowSnapshot, EventDispatcher and its listeners. "
            "Only the data files and external-service adapters differ.", BLUE);
    boxBottom(BLUE);
    pause();
}

void Tui::startInstanceScreen() {
    const size_t index = current;
    const Scenario &scenario = scenarios[index];
    beginScreen(scenario.industry + "  ›  Start a workflow instance");

    // Starting is work too: only the role the scenario names may do it.
    if (!scenario.starterRole.empty() && !currentActor()->hasRole(scenario.starterRole)) {
        noticeError("Starting this workflow needs the role " + scenario.starterRole + ", and you are " + people[actor].roles + ".");
        return;
    }

    boxTop("New instance, started by " + people[actor].name, GREEN);
    boxLine(fg(GREY, "The engine gives it an instance id. Press Enter on an empty line to go back."), GREEN);
    boxBottom(GREEN);

    InstanceData data;
    std::string summary = "Started by " + people[actor].name;
    data.set("startedBy", people[actor].name);
    for (const FieldPrompt &field : scenario.startFields) {
        std::string value;
        if (!ask(field.label, value)) {
            return;
        }
        if (value.empty()) {
            noticeWarning("Not started: " + field.label + " was left empty.");
            return;
        }
        data.set(field.key, value);
        summary += ", " + value;
    }

    const std::string id = facade.startInstance(scenario.workflowId, data, scenario.roster);
    if (id.empty()) {
        noticeError("The engine could not start this instance: " + facade.getLastError());
        return;
    }

    InstanceRecord record;
    record.id = id;
    record.summary = summary;
    record.scenario = index;
    instances.push_back(record);

    // The instance's own id belongs with the details entered for it.
    facade.getInstance(id)->getData().set("instanceId", id);

    HistoryEntry started;
    started.instanceId = id;
    started.when = std::time(nullptr);
    started.what = "CREATED";
    started.detail = summary;
    history.push_back(started);

    noticeOk("Started " + id + " by " + people[actor].name);
    flushEvents();
    instanceScreen(record);
}

void Tui::openInstanceScreen() {
    const std::vector<const InstanceRecord *> mine = currentInstances();
    beginScreen(scenarios[current].industry + "  ›  Instances");
    if (mine.empty()) {
        boxTop("Nothing started yet", GOLD);
        boxLine("Choose 2 on the main menu to start your first instance.", GOLD);
        boxBottom(GOLD);
        pause();
        return;
    }

    boxTop("Choose an instance", SKY);
    for (size_t i = 0; i < mine.size(); ++i) {
        int done = 0;
        int total = 0;
        taskProgress(mine[i]->id, done, total);
        boxLine(bold(SKY, pad(" " + std::to_string(i + 1), 4)) + pad(bold(WHITE, mine[i]->id), 13) +
                    progressBar(done, total, 12) + " " + pad(std::to_string(done) + "/" + std::to_string(total), 6) +
                    pad(statusPill(facade.getStatus(mine[i]->id)), 14) + fg(GREY, fit(mine[i]->summary, screenWidth() - 54)),
                SKY);
    }
    boxLine(bold(RED, pad(" 0", 4)) + bold(WHITE, "Back"), SKY);
    boxBottom(SKY);

    const int choice = askNumber("Choose", 0, static_cast<int>(mine.size()));
    if (choice > 0) {
        const InstanceRecord record = *mine[static_cast<size_t>(choice - 1)];
        instanceScreen(record);
    }
}

void Tui::taskProgress(const std::string &instanceId, int &done, int &total) const {
    std::vector<Row> rows;
    collectRows(facade.getInstance(instanceId)->getRoot(), "", true, true, rows);
    done = 0;
    total = 0;
    for (const Row &row : rows) {
        if (taskOf(row.item) == nullptr) continue;
        ++total;
        if (isFinal(row.item->getStateName())) ++done;
    }
}

void Tui::instanceScreen(const InstanceRecord &record) {
    const Scenario &scenario = scenarios[record.scenario];

    while (!inputClosed) {
        std::vector<Row> rows;
        WorkflowInstance *instance = facade.getInstance(record.id);
        collectRows(instance->getRoot(), "", true, true, rows);

        int done = 0;
        int total = 0;
        taskProgress(record.id, done, total);

        beginScreen(record.id + "  ›  " + scenario.industry);

        boxTop(scenario.process, BLUE);
        boxLine(fg(GREY, fit(record.summary, screenWidth() - 4)), BLUE);
        boxLine(statusPill(instance->status()) + "   " + progressBar(done, total, 24) + " " + bold(WHITE, std::to_string(done) + "/" + std::to_string(total)) + fg(GREY, " tasks finished"), BLUE);
        boxLine("Acting as " + actorLabel(), BLUE);
        boxLine("", BLUE);

        const size_t outcomeWidth = screenWidth() - 4 - 2 - 4 - 38 - 14 - 21;
        boxLine(bold(GREY, "  " + pad("#", 4) + pad("Work item", 38) + pad("State", 14) + pad("Assignee", 21) + "Outcome"), BLUE);
        for (size_t i = 0; i < rows.size(); ++i) {
            WorkItem *item = rows[i].item;
            const bool composite = taskOf(item) == nullptr;
            const bool mine = !composite && !facade.availableActions(record.id, item->getId(), currentActor()).empty();
            const int priority = priorityOf(item);

            std::string name = fg(240, rows[i].branch);
            name += composite ? bold(VIOLET, item->getName()) : (mine ? bold(WHITE, item->getName()) : item->getName());
            if (priority > 0) name += " " + bold(GOLD, "★" + std::to_string(priority));

            const std::string assignee = item->getAssignee() ? fit(item->getAssignee()->getName(), 20) : "";
            boxLine((mine ? bold(PINK, "▸ ") : "  ") + bold(SKY, pad(std::to_string(i + 1), 4)) + pad(name, 38) +
                        stateCell(item->getStateName(), 14) + pad(assignee.empty() ? fg(240, "-") : assignee, 21) +
                        fg(GREY, fit(shownOutcome(item), outcomeWidth)),
                    BLUE);
        }
        boxLine("", BLUE);
        boxLine(bold(PINK, "▸") + fg(GREY, " you can act on this now    ") + bold(GOLD, "★") + fg(GREY, " priority    ") + bold(VIOLET, "purple") + fg(GREY, " stages contain tasks"), BLUE);
        boxBottom(BLUE);

        std::cout << fg(GREY, "  Number") << " open a work item  " << bold(SKY, "h") << " history  " << bold(SKY, "d") << " data  " << bold(SKY, "c") << " commands  "
                  << bold(SKY, "p") << " change participant  " << bold(SKY, "u") << " undo  " << bold(SKY, "b") << " back\n";

        std::string answer;
        if (!ask("Choose", answer)) {
            return;
        }

        if (answer == "b" || answer == "0") return;
        if (answer == "h") { showInstanceHistory(record.id); continue; }
        if (answer == "d") { showInstanceData(record.id); continue; }
        if (answer == "c") { showCommands(record.id); continue; }
        if (answer == "p") { chooseActor(); continue; }
        if (answer == "u") { undoLast(); continue; }

        char *end = nullptr;
        const long number = std::strtol(answer.c_str(), &end, 10);
        if (answer.empty() || *end != '\0' || number < 1 || number > static_cast<long>(rows.size())) {
            noticeWarning("That is not one of the options.");
            continue;
        }
        itemScreen(record, rows[static_cast<size_t>(number - 1)].item->getId());
    }
}

void Tui::itemScreen(const InstanceRecord &record, const std::string &itemId) {
    WorkflowInstance *instance = facade.getInstance(record.id);
    WorkItem *item = instance->findItem(itemId);
    if (item == nullptr) {
        return;
    }
    Task *task = taskOf(item);

    beginScreen(record.id + "  ›  " + item->getName());

    const int colour = stateColour(item->getStateName());
    boxTop(item->getName() + " (" + item->getId() + ")", colour);
    boxLine(pad(bold(GREY, "State"), 12) + stateCell(item->getStateName(), 14), colour);
    boxLine(pad(bold(GREY, "Assignee"), 12) + (item->getAssignee() ? item->getAssignee()->getName() : std::string("-")), colour);
    if (!shownOutcome(item).empty()) {
        boxLine(pad(bold(GREY, "Outcome"), 12) + fit(shownOutcome(item), screenWidth() - 18), colour);
    }
    if (priorityOf(item) > 0) {
        boxLine(pad(bold(GREY, "Priority"), 12) + bold(GOLD, "★ " + std::to_string(priorityOf(item))), colour);
    }
    if (task != nullptr && task->getDeadlineHours() > 0) {
        boxLine(pad(bold(GREY, "Deadline"), 12) + std::to_string(task->getDeadlineHours()) + " hours", colour);
    }
    const std::string role = requiredRole(task, instance->getData());
    if (!role.empty()) {
        boxLine(pad(bold(GREY, "Role"), 12) + bold(PINK, role), colour);
    }
    const ApprovalRouting *approval = task != nullptr ? dynamic_cast<const ApprovalRouting *>(task->getRouting()) : nullptr;
    if (approval != nullptr) {
        const std::vector<std::string> levels = approval->describeChain();
        for (size_t i = 0; i < levels.size(); ++i) {
            const bool here = static_cast<int>(i) == item->getEscalationLevel();
            boxLine(pad(i == 0 ? bold(GREY, "Approval") : std::string(), 12) + (here ? bold(PINK, "▸ " + levels[i]) : fg(GREY, "  " + levels[i])), colour);
        }
    } else if (item->getEscalationLevel() > 0) {
        boxLine(pad(bold(GREY, "Escalated"), 12) + bold(ORANGE, std::to_string(item->getEscalationLevel()) + " time(s)"), colour);
    }
    if (task != nullptr && !task->getServiceName().empty()) {
        boxLine(pad(bold(GREY, "Service"), 12) + task->getServiceName() + fg(GREY, "  (called when you complete this)"), colour);
    }
    boxBottom(colour);
    std::cout << "\n";

    const std::vector<std::string> allowed = facade.availableActions(record.id, itemId, currentActor());
    const struct { const char *key; const char *name; } actions[] = {
        {"claim", "Claim"}, {"start", "Start work"}, {"complete", "Complete"},
        {"reject", "Reject"}, {"delegate", "Delegate to someone else"}, {"cancel", "Cancel"},
    };

    boxTop("Actions", PINK);
    boxLine("Acting as " + actorLabel(), PINK);
    boxLine("", PINK);
    int number = 1;
    for (const auto &action : actions) {
        const bool can = std::find(allowed.begin(), allowed.end(), action.key) != allowed.end();
        const std::string key = " " + std::to_string(number++) + " ";
        boxLine((can ? bold(GREEN, key) + " " + pad(bold(WHITE, action.name), 28) + bold(GREEN, "✓ allowed")
                     : fg(240, key) + " " + pad(fg(GREY, action.name), 28) + fg(240, "the engine will refuse this")),
                PINK);
    }
    const bool canSwitch = item->getAssignee() != nullptr && item->getAssignee()->getId() != people[actor].id;
    if (canSwitch) {
        boxLine(bold(PINK, " 7 ") + " " + bold(WHITE, "Act as the assignee") + fg(GREY, " (" + item->getAssignee()->getName() + ")"), PINK);
    }
    boxLine(bold(RED, " 0 ") + " " + bold(WHITE, "Back"), PINK);
    boxBottom(PINK);

    const int choice = askNumber("Choose", 0, canSwitch ? 7 : 6);
    if (choice <= 0) {
        return;
    }
    if (choice == 7) {
        actor = personIndex(item->getAssignee()->getId());
        itemScreen(record, itemId);
        return;
    }
    perform(record, item, choice);
}

void Tui::perform(const InstanceRecord &record, WorkItem *item, int choice) {
    Participant *who = currentActor();
    const std::string id = item->getId();
    const std::string by = " by " + people[actor].name;
    const std::string needed = requiredRole(taskOf(item), facade.getInstance(record.id)->getData());
    bool accepted = false;
    std::string description;
    std::string hint;

    switch (choice) {
        case 1:
            accepted = facade.claim(record.id, id, who);
            description = "Claimed " + item->getName() + by;
            break;
        case 2:
            accepted = facade.start(record.id, id, who);
            description = "Started " + item->getName() + by;
            break;
        case 3: {
            std::string outcome;
            const Scenario &scenario = scenarios[record.scenario];
            const std::vector<std::string> allowed = facade.availableActions(record.id, id, who);
            const std::map<std::string, std::vector<FieldPrompt>>::const_iterator asked = scenario.taskFields.find(id);
            // Ask only when the engine will accept the completion, so a refusal changes no data.
            if (asked != scenario.taskFields.end() && std::find(allowed.begin(), allowed.end(), "complete") != allowed.end()) {
                InstanceData &data = facade.getInstance(record.id)->getData();
                for (const FieldPrompt &field : asked->second) {
                    std::string value;
                    const std::string current = data.get(field.key);
                    if (!ask(field.label + (current.empty() ? "" : " [" + current + "]"), value)) return;
                    if (value.empty()) value = current;
                    if (value.empty()) {
                        noticeWarning("Not completed: " + field.label + " was left empty.");
                        return;
                    }
                    data.set(field.key, value);
                }
            }
            const std::map<std::string, std::vector<std::string>>::const_iterator choices = scenario.outcomes.find(id);
            if (choices == scenario.outcomes.end()) {
                if (!ask("Outcome [done]", outcome)) return;
                if (outcome.empty()) outcome = "done";
            } else {
                for (size_t i = 0; i < choices->second.size(); ++i) {
                    std::cout << "  " << bold(GREEN, " " + std::to_string(i + 1) + " ") << " " << bold(WHITE, choices->second[i]) << "\n";
                }
                const int picked = askNumber("Outcome", 1, static_cast<int>(choices->second.size()));
                if (picked < 1) return;
                outcome = choices->second[static_cast<size_t>(picked - 1)];
            }
            accepted = facade.complete(record.id, id, who, outcome);
            break;
        }
        case 4: {
            std::string reason;
            if (!ask("Reason for rejecting", reason)) return;
            if (reason.empty()) {
                noticeWarning("A rejection needs a reason.");
                return;
            }
            accepted = facade.reject(record.id, id, who, reason);
            description = "Rejected " + item->getName() + " (" + reason + ")" + by;
            break;
        }
        case 5: {
            const std::vector<size_t> roster = printRoster();
            const int picked = askNumber("Delegate to", 1, static_cast<int>(roster.size()));
            if (picked < 1) return;
            const Person &target = people[roster[static_cast<size_t>(picked - 1)]];
            accepted = facade.delegate(record.id, id, who, facade.findParticipant(target.id, scenarios[current].roster));
            description = "Delegated " + item->getName() + " to " + target.name + by;
            if (!accepted && target.id == people[actor].id) {
                hint = "You cannot delegate work to yourself.";
            } else if (!accepted && !facade.findParticipant(target.id, scenarios[current].roster)->hasRole(needed) && !needed.empty()) {
                hint = "This work needs the role " + needed + ", and " + target.name + " holds " + target.roles + ".";
            }
            break;
        }
        case 6:
            accepted = facade.cancel(record.id, id, who);
            description = "Cancelled " + item->getName() + by;
            break;
        default:
            return;
    }

    if (!accepted) {
        noticeError("Refused by the engine: " + facade.getLastError());
        const std::string assignee = item->getAssignee() ? "assigned to " + item->getAssignee()->getName() : "not assigned to anyone";
        notices.push_back(fg(GREY, "  " + item->getName() + " is " + item->getStateName() + " and " + assignee + "."));
        if (hint.empty() && !needed.empty() && !who->hasRole(needed)) {
            hint = "This work needs the role " + needed + ", and you are " + people[actor].roles + ".";
        }
        if (!hint.empty()) {
            notices.push_back(fg(GREY, "  " + hint));
        }
        return;
    }

    // A completion may have called an external service; show what it said.
    const Task *task = taskOf(item);
    const std::map<std::string, std::string> &data = facade.getInstance(record.id)->getData().getMap();
    const std::map<std::string, std::string>::const_iterator reply = data.find("service." + id + ".message");
    const bool serviceFailed = choice == 3 && item->getStateName() == "Escalated";
    if (serviceFailed) {
        noticeWarning(item->getName() + " could not finish, so it escalated.");
    } else if (choice == 3 && item->getStateName() == "Rejected") {
        noticeWarning(item->getName() + " was rejected by the service's answer.");
    } else if (choice == 3) {
        // A service can decide the outcome, so report what the engine recorded, not what was typed.
        noticeOk("Completed " + item->getName() + " (" + item->getOutcome() + ")" + by);
    } else {
        noticeOk(description);
    }
    if (choice == 3 && task != nullptr && !task->getServiceName().empty() && reply != data.end()) {
        notices.push_back(bold(VIOLET, "⇄ " + task->getServiceName() + ": ") + (serviceFailed ? bold(RED, reply->second) : reply->second));
    }
    flushEvents();
}

void Tui::myWorkScreen() {
    struct WorkRow {
        const InstanceRecord *record;
        WorkItem *item;
    };

    std::vector<WorkRow> work;
    for (const InstanceRecord *record : currentInstances()) {
        std::vector<Row> rows;
        collectRows(facade.getInstance(record->id)->getRoot(), "", true, true, rows);
        for (const Row &row : rows) {
            if (taskOf(row.item) != nullptr && !facade.availableActions(record->id, row.item->getId(), currentActor()).empty()) {
                WorkRow entry;
                entry.record = record;
                entry.item = row.item;
                work.push_back(entry);
            }
        }
    }

    // Most urgent first; PriorityDecorator sets the level.
    std::stable_sort(work.begin(), work.end(), [](const WorkRow &a, const WorkRow &b) { return priorityOf(a.item) > priorityOf(b.item); });

    beginScreen("My work");
    boxTop("Work for " + people[actor].name, GOLD);
    if (work.empty()) {
        boxLine("Nothing for you right now. Change participant (5) to work as someone else.", GOLD);
        boxBottom(GOLD);
        pause();
        return;
    }
    boxLine(bold(GREY, pad(" #", 4) + pad("Instance", 13) + pad("Work item", 34) + pad("State", 14) + "Assignee"), GOLD);
    for (size_t i = 0; i < work.size(); ++i) {
        WorkItem *item = work[i].item;
        std::string name = item->getName();
        if (priorityOf(item) > 0) name += " " + bold(GOLD, "★" + std::to_string(priorityOf(item)));
        boxLine(bold(GOLD, pad(" " + std::to_string(i + 1), 4)) + pad(work[i].record->id, 13) + pad(name, 34) +
                    stateCell(item->getStateName(), 14) + (item->getAssignee() ? bold(GREEN, "you") : bold(ORANGE, "unclaimed")),
                GOLD);
    }
    boxLine(bold(RED, pad(" 0", 4)) + bold(WHITE, "Back"), GOLD);
    boxBottom(GOLD);

    const int choice = askNumber("Choose", 0, static_cast<int>(work.size()));
    if (choice > 0) {
        const WorkRow &row = work[static_cast<size_t>(choice - 1)];
        itemScreen(*row.record, row.item->getId());
    }
}

/// Lists the current scenario's participants. @return Their positions in people, in menu order.
std::vector<size_t> Tui::printRoster() {
    std::vector<size_t> roster;
    boxTop(scenarios[current].industry + " participants", PINK);
    for (size_t i = 0; i < people.size(); ++i) {
        if (people[i].scenario != current) continue;
        roster.push_back(i);
        boxLine(bold(PINK, pad(" " + std::to_string(roster.size()), 4)) +
                    pad(bold(WHITE, people[i].name), 22) + fg(GREY, people[i].roles),
                PINK);
    }
    boxLine(bold(RED, pad(" 0", 4)) + bold(WHITE, "Back"), PINK);
    boxBottom(PINK);
    return roster;
}

/// @return true if a participant was chosen; false if the user went back.
bool Tui::chooseActor() {
    beginScreen(scenarios[current].industry + "  ›  Who are you?");
    const std::vector<size_t> roster = printRoster();
    const int choice = askNumber("Act as", 0, static_cast<int>(roster.size()));
    if (choice < 1) {
        return false;
    }
    actor = roster[static_cast<size_t>(choice - 1)];
    noticeOk("Now acting as " + people[actor].name);
    return true;
}

void Tui::undoLast() {
    const std::vector<std::string> actions = facade.actionHistory();
    if (actions.empty()) {
        noticeWarning("There is nothing to undo.");
        return;
    }

    // Each entry reads "<instance id>: <what it was>".
    const std::string entry = actions.back();
    const size_t colon = entry.find(": ");
    const std::string instanceId = entry.substr(0, colon);
    const std::string what = colon == std::string::npos ? entry : entry.substr(colon + 2);

    if (!facade.undoLast()) {
        noticeError("The engine could not undo that action: " + facade.getLastError());
        return;
    }

    HistoryEntry row;
    row.instanceId = instanceId;
    row.when = std::time(nullptr);
    row.what = "UNDONE";
    row.detail = what;
    history.push_back(row);

    noticeOk("Undone in " + instanceId + ": " + what);
    notices.push_back(fg(GREY, "Restored from a snapshot. External service calls cannot be recalled."));
}

void Tui::showInstanceHistory(const std::string &instanceId) {
    beginScreen("History of " + instanceId);
    boxTop("Everything that happened, oldest first", SKY);
    for (const HistoryEntry &row : history) {
        if (row.instanceId != instanceId) continue;
        const int colour = row.what == "UNDONE" ? PINK : VIOLET;
        boxLine(fg(GREY, clock(row.when)) + "  " + pad(bold(colour, row.what), 11) + pad(fit(row.itemName, 30), 31) +
                    fg(GREY, fit(row.detail, screenWidth() - 57)),
                SKY);
    }
    boxBottom(SKY);
    pause();
}

void Tui::showInstanceData(const std::string &instanceId) {
    beginScreen("Data of " + instanceId);
    const std::map<std::string, std::string> &data = facade.getInstance(instanceId)->getData().getMap();

    boxTop("Instance data", GREEN);
    for (const auto &entry : data) {
        if (entry.first.compare(0, 8, "service.") != 0) {
            boxLine(pad(bold(WHITE, entry.first), 22) + entry.second, GREEN);
        }
    }
    boxBottom(GREEN);

    boxTop("External service replies", VIOLET);
    bool anyReply = false;
    for (const auto &entry : data) {
        if (entry.first.compare(0, 8, "service.") == 0) {
            boxLine(pad(bold(WHITE, entry.first.substr(8)), 30) + fit(entry.second, screenWidth() - 36), VIOLET);
            anyReply = true;
        }
    }
    if (!anyReply) boxLine(fg(GREY, "No services have been called yet."), VIOLET);
    boxBottom(VIOLET);
    pause();
}

/// The engine's own record of commands that ran and can still be undone, oldest first.
void Tui::showCommands(const std::string &instanceId) {
    beginScreen("Commands of " + instanceId);
    boxTop("Commands the engine has run, oldest first", SKY);
    const std::string prefix = instanceId + ": ";
    int shown = 0;
    for (const std::string &entry : facade.actionHistory()) {
        if (entry.compare(0, prefix.size(), prefix) == 0) {
            boxLine(bold(SKY, pad(std::to_string(++shown), 4)) + fit(entry.substr(prefix.size()), screenWidth() - 10), SKY);
        }
    }
    if (shown == 0) boxLine(fg(GREY, "No commands yet."), SKY);
    boxLine("", SKY);
    boxLine(fg(GREY, "Refused actions are not kept. u undoes the newest command across all workflows."), SKY);
    boxBottom(SKY);
    pause();
}

void Tui::showAuditTrail() {
    beginScreen("Audit trail");
    if (audit->getEntryCount() == 0) {
        boxTop("Nothing yet", GOLD);
        boxLine("Nothing has happened yet.", GOLD);
        boxBottom(GOLD);
        pause();
        return;
    }
    for (const std::string &entry : audit->getAuditTrail()) {
        std::cout << fg(GREY, entry) << "\n";
    }
    pause();
}

void Tui::showMetrics() {
    beginScreen("Monitoring metrics");

    int running = 0;
    int completed = 0;
    int terminated = 0;
    for (const InstanceRecord &record : instances) {
        const std::string status = facade.getStatus(record.id);
        if (status == "completed") ++completed;
        else if (status == "terminated") ++terminated;
        else ++running;
    }

    boxTop("Instances", SKY);
    boxLine(bold(BLUE, std::to_string(running) + " running") + "    " + bold(GREEN, std::to_string(completed) + " completed") + "    " + bold(RED, std::to_string(terminated) + " terminated"), SKY);
    boxBottom(SKY);

    boxTop("Events seen by the observers", VIOLET);
    const EventType types[] = {EventType::AVAILABLE, EventType::ASSIGNED, EventType::STARTED, EventType::COMPLETED,
                               EventType::REJECTED, EventType::CANCELLED, EventType::ESCALATED};
    size_t most = 1;
    for (EventType type : types) most = std::max(most, monitor->getCountForType(type));
    for (EventType type : types) {
        const size_t count = monitor->getCountForType(type);
        boxLine(pad(bold(eventColour(type), eventTypeToString(type)), 12) + pad(std::to_string(count), 5) + fg(eventColour(type), repeat("█", count * 50 / most)), VIOLET);
    }
    boxLine("", VIOLET);
    std::ostringstream failures;
    failures.precision(1);
    failures << std::fixed << monitor->getFailureRate();
    boxLine(fg(GREY, "Total ") + bold(WHITE, std::to_string(monitor->getTotalEvents())) + fg(GREY, "    Failure rate ") + bold(WHITE, failures.str() + "%") +
                fg(GREY, "    Notifications ") + bold(WHITE, std::to_string(notifier->getNotificationCount())),
            VIOLET);
    boxBottom(VIOLET);
    pause();
}

void Tui::checkDeadlines() {
    beginScreen("Check deadlines");
    boxTop("Jump ahead in time", ORANGE);
    boxText("Work with a deadline escalates once it passes. Deadlines run in hours, so choose how far ahead to check. Nothing else moves.", ORANGE);
    boxLine("", ORANGE);
    const std::time_t now = std::time(nullptr);
    bool anyDue = false;
    for (const InstanceRecord &record : instances) {
        WorkflowInstance *instance = facade.getInstance(record.id);
        std::vector<Row> rows;
        collectRows(instance->getRoot(), "", true, true, rows);
        for (const Row &row : rows) {
            const std::time_t due = instance->getDeadline(row.item->getId());
            if (due == 0) continue;
            // Round up, so the number shown is enough to reach the deadline.
            const long left = (static_cast<long>(due - now) + 3599) / 3600;
            boxLine(pad(record.id, 13) + pad(bold(WHITE, row.item->getName()), 30) + bold(ORANGE, "due in " + std::to_string(left) + " hours"), ORANGE);
            anyDue = true;
        }
    }
    if (!anyDue) boxLine(fg(GREY, "Nothing open has a deadline right now."), ORANGE);
    boxBottom(ORANGE);

    std::string answer;
    if (!ask("Hours from now [0]", answer)) {
        return;
    }
    char *end = nullptr;
    const long hours = answer.empty() ? 0 : std::strtol(answer.c_str(), &end, 10);
    if (!answer.empty() && (*end != '\0' || hours < 0 || hours > 100000)) {
        noticeWarning("Please enter a whole number of hours.");
        return;
    }

    facade.checkDeadlines(std::time(nullptr) + hours * 3600);

    if (events->empty()) {
        noticeOk("No deadlines have passed " + std::to_string(hours) + " hours from now.");
        return;
    }
    checkingDeadlines = true;
    flushEvents();
    checkingDeadlines = false;
}

// ---- Helpers --------------------------------------------------------------------------------

/// Shows what the last action caused, and keeps it as history.
void Tui::flushEvents() {
    for (const WorkflowEvent &event : events->take()) {
        const bool hasOutcome = event.type == EventType::COMPLETED || event.type == EventType::REJECTED ||
                                event.type == EventType::CANCELLED || event.type == EventType::ESCALATED;
        std::string detail = hasOutcome ? event.outcome : "";

        WorkflowInstance *instance = facade.getInstance(event.instanceId);
        WorkItem *item = instance != nullptr ? instance->findItem(event.itemId) : nullptr;
        if (event.type == EventType::ASSIGNED && item != nullptr && item->getAssignee() != nullptr) {
            detail = "-> " + item->getAssignee()->getName();
        }
        if (event.type == EventType::ESCALATED && detail.empty()) {
            const InstanceData *data = instance != nullptr ? &instance->getData() : nullptr;
            const std::string service = "service." + event.itemId + ".";
            if (checkingDeadlines) {
                detail = "deadline passed";
            } else if (data != nullptr && data->get(service + "success") == "false") {
                detail = data->get(service + "message");
            } else {
                detail = "no one in this scenario holds the role";
            }
        }

        HistoryEntry row;
        row.instanceId = event.instanceId;
        row.when = event.timestamp;
        row.itemName = event.itemName;
        row.what = eventTypeToString(event.type);
        row.detail = detail;
        history.push_back(row);

        notices.push_back(pad(bold(eventColour(event.type), row.what), 11) + pad(fit(row.itemName, 32), 33) + fg(GREY, fit(detail, screenWidth() - 50)));
    }
}

void Tui::noticeOk(const std::string &text) {
    notices.push_back(bold(GREEN, "✓ ") + fit(text, screenWidth() - 7));
}

void Tui::noticeWarning(const std::string &text) {
    notices.push_back(bold(ORANGE, "▲ ") + fit(text, screenWidth() - 7));
}

void Tui::noticeError(const std::string &text) {
    // Refusals explain themselves, so they wrap rather than get cut off.
    const std::vector<std::string> lines = wrap(text, screenWidth() - 7);
    for (size_t i = 0; i < lines.size(); ++i) {
        notices.push_back((i == 0 ? bold(RED, "✗ ") : std::string("  ")) + bold(RED, lines[i]));
    }
}

size_t Tui::personIndex(const std::string &id) const {
    for (size_t i = 0; i < people.size(); ++i) {
        if (people[i].id == id && people[i].scenario == current) {
            return i;
        }
    }
    return actor;
}

Participant *Tui::currentActor() const {
    return facade.findParticipant(people[actor].id, scenarios[people[actor].scenario].roster);
}

std::vector<const InstanceRecord *> Tui::currentInstances() const {
    std::vector<const InstanceRecord *> mine;
    for (const InstanceRecord &record : instances) {
        if (record.scenario == current) mine.push_back(&record);
    }
    return mine;
}

std::string Tui::actorLabel() const {
    return bold(PINK, people[actor].name) + fg(GREY, "  " + people[actor].roles);
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
