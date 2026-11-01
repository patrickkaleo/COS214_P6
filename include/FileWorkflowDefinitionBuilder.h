#ifndef FILEWORKFLOWDEFINITIONBUILDER_H
#define FILEWORKFLOWDEFINITIONBUILDER_H

#include "DataRuleRouting.h"
#include "RoleBasedRouting.h"

#include <fstream>
#include <map>
#include <sstream>

#include "WorkflowDefinitionBuilder.h"

/**
 * @class FileWorkflowDefinitionBuilder
 * @brief Concrete builder that reads one sectioned workflow text file.
 *
 * Lines starting with # above the first section are the definition's description; a bare #
 * separates lines of it. Blank lines and any later # lines are ignored. A process whose parent
 * id is "-" becomes the root. A run starts from that root.
 *
 * @code
 * DEFINITION
 * UNI_APP|University of pretoria|1
 *
 * WORKITEMS
 * process|INTAKE|-|Application Intake
 * task|CAPTURE|INTAKE|Capture Application
 *
 * DEPENDENCIES
 * DECISION|ISSUE_OFFER|approved
 *
 * ROUTING
 * CAPTURE|role|Admissions Officer
 * CLASSIFY|data|severity|Exception Analyst|low|Warehouse Clerk|high|Exception Analyst
 *
 * SERVICES
 * service|VERIFY_DOCS|Document Verification Service
 *
 * EXTRAS
 * checkpoint|DECISION
 * priority|FACULTY_REVIEW|2
 * audit|DECISION
 * deadline|DECISION|24
 * @endcode
 */
class FileWorkflowDefinitionBuilder : public WorkflowDefinitionBuilder
{
public:
	/**
	 * @brief Read the file into sections. The director applies each section in order.
	 * @param filePath path to the workflow text file
	 */
	explicit FileWorkflowDefinitionBuilder(string filePath);
	FileWorkflowDefinitionBuilder() = delete;
	FileWorkflowDefinitionBuilder(const FileWorkflowDefinitionBuilder &) = delete;
	FileWorkflowDefinitionBuilder &operator=(const FileWorkflowDefinitionBuilder &) = delete;

	/**
	 * @brief Read DEFINITION and build the work-item tree from WORKITEMS.
	 * Malformed records throw std::invalid_argument; unreadable files throw
	 * std::runtime_error. The builder retains ownership if a build step fails.
	 */
	void buildStructure() override;

	/**
	 * @brief Add the rules listed under DEPENDENCIES.
	 */
	void buildDependencies() override;

	/**
	 * @brief Attach the services listed under SERVICES.
	 */
	void buildServices() override;

	/**
	 * @brief Attach the routing strategies listed under ROUTING.
	 */
	void buildRouting() override;

	/**
	 * @brief Apply checkpoints, priorities, audits and deadlines listed under EXTRAS.
	 * Deadline hours must be an integer from 1 to 87600.
	 */
	void buildExtras() override;

private:
	/**
	 * @brief Path of the workflow text file passed to the constructor.
	 */
	string filePath;

	/**
	 * @brief True after the file has been split into sections.
	 */
	bool loaded;

	string description;
	vector<string> definitionLines;
	vector<string> workItemLines;
	vector<string> dependencyLines;
	vector<string> routingLines;
	vector<string> serviceLines;
	vector<string> extraLines;

	/**
	 * @brief Split the file into the definition header and each build section.
	 */
	void readFile();
	void invalidRecord(const string &section, size_t index, const string &reason) const;
};

#endif
