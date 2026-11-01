#include "FileWorkflowDefinitionBuilder.h"
#include <set>
#include <stdexcept>

namespace {
string trim(const string &text) {
    const size_t start = text.find_first_not_of(" \t\r");
    if (start == string::npos) return "";
    return text.substr(start, text.find_last_not_of(" \t\r") - start + 1);
}
vector<string> fieldsOf(const string &line) {
    vector<string> fields;
    size_t start = 0;
    for (;;) {
        const size_t end = line.find('|', start);
        fields.push_back(trim(line.substr(start, end == string::npos ? end : end - start)));
        if (end == string::npos) return fields;
        start = end + 1;
    }
}
string trimDescription(const string &text) {
    const size_t start = text.find_first_not_of("\n");
    if (start == string::npos) return "";
    return text.substr(start, text.find_last_not_of("\n") - start + 1);
}
bool completeFields(const vector<string> &fields, size_t count) {
    if (fields.size() != count) return false;
    for (size_t i = 0; i < fields.size(); ++i)
        if (fields[i].empty()) return false;
    return true;
}
bool integerOf(const string &text, int &value) {
    istringstream number(text);
    if (!(number >> value)) return false;
    number >> ws;
    return number.eof();
}
}

FileWorkflowDefinitionBuilder::FileWorkflowDefinitionBuilder(string path)
    : filePath(path), loaded(false) { readFile(); }

void FileWorkflowDefinitionBuilder::invalidRecord(
    const string &section, size_t index, const string &reason) const {
    throw invalid_argument(filePath + ": " + section + " record " +
        to_string(index + 1) + ": " + reason);
}

void FileWorkflowDefinitionBuilder::readFile() {
    description.clear();
    definitionLines.clear();
    workItemLines.clear();
    dependencyLines.clear();
    routingLines.clear();
    serviceLines.clear();
    extraLines.clear();
    loaded = false;
    ifstream file(filePath.c_str());
    if (!file) throw runtime_error("Cannot read workflow file: " + filePath);

    string section, line;
    set<string> sections;
    size_t lineNumber = 0;
    while (getline(file, line)) {
        ++lineNumber;
        line = trim(line);
        if (line[0] == '#' && section.empty()) {
            description += trim(line.substr(1)) + "\n";
            continue;
        }
        if (line.empty() || line[0] == '#') continue;
        if (line == "DEFINITION" || line == "WORKITEMS" || line == "DEPENDENCIES" ||
            line == "ROUTING" || line == "SERVICES" || line == "EXTRAS") {
            if (!sections.insert(line).second)
                throw invalid_argument(filePath + ": line " + to_string(lineNumber) +
                    ": duplicate section " + line);
            section = line;
            continue;
        }
        if (section.empty())
            throw invalid_argument(filePath + ": line " + to_string(lineNumber) +
                ": expected a section heading");
        if (section == "DEFINITION") definitionLines.push_back(line);
        else if (section == "WORKITEMS") workItemLines.push_back(line);
        else if (section == "DEPENDENCIES") dependencyLines.push_back(line);
        else if (section == "ROUTING") routingLines.push_back(line);
        else if (section == "SERVICES") serviceLines.push_back(line);
        else if (section == "EXTRAS") extraLines.push_back(line);
    }
    if (file.bad()) throw runtime_error("Could not finish reading workflow file: " + filePath);
    description = trimDescription(description);
    loaded = true;
}

void FileWorkflowDefinitionBuilder::buildStructure() {
    if (!loaded) readFile();
    if (definitionLines.size() != 1)
        invalidRecord("DEFINITION", 0, "exactly one id|name|version record is required");
    const vector<string> header = fieldsOf(definitionLines[0]);
    int version = 0;
    if (!completeFields(header, 3) || !integerOf(header[2], version) || version < 1)
        invalidRecord("DEFINITION", 0, "expected id|name|positive integer version");

    vector<vector<string> > records;
    map<string, size_t> identifiers;
    size_t roots = 0;
    for (size_t i = 0; i < workItemLines.size(); ++i) {
        const vector<string> fields = fieldsOf(workItemLines[i]);
        if (!completeFields(fields, 4) || (fields[0] != "process" && fields[0] != "task"))
            invalidRecord("WORKITEMS", i, "expected process|id|parent|name or task|id|parent|name");
        if (fields[1] == "-" || !identifiers.insert(make_pair(fields[1], i)).second)
            invalidRecord("WORKITEMS", i, "duplicate or reserved item id: " + fields[1]);
        if (fields[2] == "-") {
            if (fields[0] != "process") invalidRecord("WORKITEMS", i, "the root must be a process");
            ++roots;
        }
        records.push_back(fields);
    }
    if (roots != 1) invalidRecord("WORKITEMS", 0, "exactly one root process is required");
    for (size_t i = 0; i < records.size(); ++i) {
        if (records[i][2] == "-") continue;
        map<string, size_t>::const_iterator parent = identifiers.find(records[i][2]);
        if (parent == identifiers.end() || records[parent->second][0] != "process")
            invalidRecord("WORKITEMS", i, "parent is not a known process: " + records[i][2]);
        if (records[i][1] == records[i][2])
            invalidRecord("WORKITEMS", i, "an item cannot be its own parent");
    }

    startDefinition(header[0], header[1], version);
    definition->setDescription(description);
    // Allow declarations in any order without ever attaching to a missing parent.
    vector<bool> built(records.size(), false);
    size_t remaining = records.size();
    while (remaining != 0) {
        bool progress = false;
        for (size_t i = 0; i < records.size(); ++i) {
            if (built[i] || (records[i][2] != "-" && findItem(records[i][2]) == nullptr)) continue;
            if (records[i][0] == "process") addProcess(records[i][1], records[i][3], records[i][2]);
            else addTask(records[i][1], records[i][3], records[i][2]);
            built[i] = true;
            --remaining;
            progress = true;
        }
        if (!progress) invalidRecord("WORKITEMS", 0, "parent relationships contain a cycle");
    }
}

void FileWorkflowDefinitionBuilder::buildDependencies() {
    if (definition == nullptr) return;
    for (size_t i = 0; i < dependencyLines.size(); ++i) {
        const vector<string> fields = fieldsOf(dependencyLines[i]);
        if (!completeFields(fields, 3)) invalidRecord("DEPENDENCIES", i, "expected fromId|toId|outcome");
        try { addDependency(fields[0], fields[1], fields[2]); }
        catch (const invalid_argument &error) { invalidRecord("DEPENDENCIES", i, error.what()); }
    }
}

void FileWorkflowDefinitionBuilder::buildServices() {
    if (definition == nullptr) return;
    set<string> servicedItems;
    for (size_t i = 0; i < serviceLines.size(); ++i) {
        const vector<string> fields = fieldsOf(serviceLines[i]);
        if (!completeFields(fields, 3) || fields[0] != "service")
            invalidRecord("SERVICES", i, "expected service|taskId|serviceName");
        if (!servicedItems.insert(fields[1]).second)
            invalidRecord("SERVICES", i, "duplicate service for item: " + fields[1]);
        try { setService(fields[1], fields[2]); }
        catch (const invalid_argument &error) { invalidRecord("SERVICES", i, error.what()); }
    }
}

void FileWorkflowDefinitionBuilder::buildRouting() {
    if (definition == nullptr) return;
    set<string> routedItems;
    for (size_t i = 0; i < routingLines.size(); ++i) {
        const vector<string> fields = fieldsOf(routingLines[i]);
        if (fields.size() < 2 || !completeFields(fields, fields.size()))
            invalidRecord("ROUTING", i, "routing fields must not be empty");
        if (!routedItems.insert(fields[0]).second)
            invalidRecord("ROUTING", i, "duplicate routing for item: " + fields[0]);
        try {
            if (fields[1] == "role" && fields.size() == 3) {
                setRouting(fields[0], new RoleBasedRouting(fields[2]));
            } else if (fields[1] == "data" && fields.size() >= 4 && fields.size() % 2 == 0) {
                map<string, string> rules;
                for (size_t pair = 4; pair < fields.size(); pair += 2) {
                    if (!rules.insert(make_pair(fields[pair], fields[pair + 1])).second)
                        throw invalid_argument("duplicate data-routing value: " + fields[pair]);
                }
                setRouting(fields[0], new DataRuleRouting(fields[2], rules, fields[3]));
            } else {
                throw invalid_argument("expected item|role|roleName or item|data|field|fallback|value|role pairs");
            }
        } catch (const invalid_argument &error) { invalidRecord("ROUTING", i, error.what()); }
    }
}

void FileWorkflowDefinitionBuilder::buildExtras() {
    if (definition == nullptr) return;
    for (size_t i = 0; i < extraLines.size(); ++i) {
        const vector<string> fields = fieldsOf(extraLines[i]);
        if (fields.size() < 2 || !completeFields(fields, fields.size()))
            invalidRecord("EXTRAS", i, "extra fields must not be empty");
        try {
            if (fields[0] == "checkpoint" && fields.size() == 2) markCheckpoint(fields[1]);
            else if (fields[0] == "audit" && fields.size() == 2) addAudit(fields[1]);
            else if (fields[0] == "deadline" && fields.size() == 3) {
                int hours = 0;
                if (!integerOf(fields[2], hours) || hours < 1 || hours > 87600)
                    throw invalid_argument("deadline must be an integer from 1 to 87600 hours");
                setDeadlineHours(fields[1], hours);
            }
            else if (fields[0] == "priority" && fields.size() == 3) {
                int level = 0;
                if (!integerOf(fields[2], level) || level < 0)
                    throw invalid_argument("priority must be a nonnegative integer");
                addPriority(fields[1], level);
            } else throw invalid_argument("unknown extra or incorrect field count");
        } catch (const invalid_argument &error) { invalidRecord("EXTRAS", i, error.what()); }
    }
}
