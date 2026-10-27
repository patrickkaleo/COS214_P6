#include "FileWorkflowDefinitionBuilder.h"

string trim(string text)
{
	while (!text.empty() && (text[text.size() - 1] == '\r' || text[text.size() - 1] == ' ' || text[text.size() - 1] == '\t'))
	{
		text.erase(text.size() - 1);
	}
	size_t start = 0;
	while (start < text.size() && (text[start] == ' ' || text[start] == '\t'))
	{
		++start;
	}
	return text.substr(start);
}

vector<string> fieldsOf(const string &line)
{
	vector<string> fields;
	string field;
	for (size_t i = 0; i < line.size(); ++i)
	{
		if (line[i] == '|')
		{
			fields.push_back(trim(field));
			field.clear();
		}
		else
		{
			field.push_back(line[i]);
		}
	}
	fields.push_back(trim(field));
	return fields;
}

bool fieldAt(const vector<string> &fields, size_t index, string &value)
{
	if (index >= fields.size() || fields[index].empty())
	{
		return false;
	}
	value = fields[index];
	return true;
}

FileWorkflowDefinitionBuilder::FileWorkflowDefinitionBuilder(string filePath)
	: filePath(filePath), loaded(false)
{
	readFile();
}

void FileWorkflowDefinitionBuilder::readFile()
{
	definitionLines.clear();
	workItemLines.clear();
	dependencyLines.clear();
	routingLines.clear();
	serviceLines.clear();
	extraLines.clear();

	// open file
	ifstream file(filePath.c_str());
	if (!file)
		return;

	string section = "";
	string line;
	while (getline(file, line))
	{
		line = trim(line);
		if (line.empty() || line[0] == '#')
			continue; // skip commented lines

		// determine where the next few lines go, which vector
		if (line == "DEFINITION" || line == "WORKITEMS" || line == "DEPENDENCIES" || line == "ROUTING" || line == "SERVICES" || line == "EXTRAS")
		{
			section = line;
			continue;
		}

		if (section == "DEFINITION")
		{
			definitionLines.push_back(line);
		}
		else if (section == "WORKITEMS")
		{
			workItemLines.push_back(line);
		}
		else if (section == "DEPENDENCIES")
		{
			dependencyLines.push_back(line);
		}
		else if (section == "ROUTING")
		{
			routingLines.push_back(line);
		}
		else if (section == "SERVICES")
		{
			serviceLines.push_back(line);
		}
		else if (section == "EXTRAS")
		{
			extraLines.push_back(line);
		}
	}
	loaded = true;
}

void FileWorkflowDefinitionBuilder::buildStructure()
{
	if (!loaded)
	{
		readFile();
	}

	string id;
	string name;
	int version = 1;
	if (!definitionLines.empty())
	{
		vector<string> fields = fieldsOf(definitionLines[0]);
		string versionText;
		if (fieldAt(fields, 0, id) && fieldAt(fields, 1, name) && fieldAt(fields, 2, versionText))
		{
			istringstream number(versionText);
			number >> version;
		}
	}
	startDefinition(id, name, version);
	if (definition == nullptr)
	{
		return;
	}

	for (size_t i = 0; i < workItemLines.size(); ++i)
	{
		vector<string> fields = fieldsOf(workItemLines[i]);
		string keyword;
		string id;
		string parentId;
		string itemName;
		if (!fieldAt(fields, 0, keyword) || !fieldAt(fields, 1, id) || !fieldAt(fields, 2, parentId) || !fieldAt(fields, 3, itemName))
		{
			continue;
		}
		if (keyword == "process")
		{
			addProcess(id, itemName, parentId);
		}
		else if (keyword == "task")
		{
			addTask(id, itemName, parentId);
		}
	}
}

void FileWorkflowDefinitionBuilder::buildDependencies()
{
	if (definition == nullptr)
	{
		return;
	}
	for (size_t i = 0; i < dependencyLines.size(); ++i)
	{
		vector<string> fields = fieldsOf(dependencyLines[i]);
		string fromId;
		string toId;
		string outcome;
		if (fieldAt(fields, 0, fromId) && fieldAt(fields, 1, toId) && fieldAt(fields, 2, outcome))
		{
			addDependency(fromId, toId, outcome);
		}
	}
}

void FileWorkflowDefinitionBuilder::buildServices()
{
	if (definition == nullptr)
	{
		return;
	}
	for (size_t i = 0; i < serviceLines.size(); ++i)
	{
		vector<string> fields = fieldsOf(serviceLines[i]);
		string keyword;
		string itemId;
		string serviceName;
		if (!fieldAt(fields, 0, keyword) || keyword != "service" || !fieldAt(fields, 1, itemId) || !fieldAt(fields, 2, serviceName))
		{
			continue;
		}
		setService(itemId, serviceName);
	}
}

void FileWorkflowDefinitionBuilder::buildRouting()
{
	if (definition == nullptr)
	{
		return;
	}
	for (size_t i = 0; i < routingLines.size(); ++i)
	{
		vector<string> fields = fieldsOf(routingLines[i]);
		string itemId;
		string kind;
		if (!fieldAt(fields, 0, itemId) || !fieldAt(fields, 1, kind))
		{
			continue;
		}
		if (kind == "role")
		{
			string roleName;
			if (fieldAt(fields, 2, roleName))
			{
				setRouting(itemId, new RoleBasedRouting(roleName));
			}
		}
		else if (kind == "data")
		{
			string field;
			string fallback;
			if (!fieldAt(fields, 2, field) || !fieldAt(fields, 3, fallback))
			{
				continue;
			}
			DataRuleRouting *routing = new DataRuleRouting(field, map<string, string>(), fallback);
			for (size_t pair = 4; pair + 1 < fields.size(); pair += 2)
			{
				string value;
				string roleName;
				if (fieldAt(fields, pair, value) && fieldAt(fields, pair + 1, roleName))
				{
					routing->addRule(value, roleName);
				}
			}
			setRouting(itemId, routing);
		}
	}
}

void FileWorkflowDefinitionBuilder::buildExtras()
{
	if (definition == nullptr)
	{
		return;
	}
	for (size_t i = 0; i < extraLines.size(); ++i)
	{
		vector<string> fields = fieldsOf(extraLines[i]);
		string keyword;
		string itemId;
		if (!fieldAt(fields, 0, keyword) || !fieldAt(fields, 1, itemId))
		{
			continue;
		}
		if (keyword == "checkpoint")
		{
			markCheckpoint(itemId);
		}
		else if (keyword == "priority")
		{
			string levelText;
			int level = 0;
			if (fieldAt(fields, 2, levelText))
			{
				istringstream number(levelText);
				if (number >> level)
				{
					addPriority(itemId, level);
				}
			}
		}
		else if (keyword == "audit")
		{
			addAudit(itemId);
		}
	}
}
