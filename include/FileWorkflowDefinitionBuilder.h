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
 * Blank lines and lines starting with # are ignored. A process whose parent id is "-"
 * becomes the root. A run starts from that root.
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
	 * Does nothing further when the file cannot be read or the id is already in use.
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
	 * @brief Apply the checkpoints, priorities and audits listed under EXTRAS.
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
};

#endif
