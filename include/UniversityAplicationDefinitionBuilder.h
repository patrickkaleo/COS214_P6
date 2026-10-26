#ifndef UNIVERSITYAPLICATIONDEFINITIONBUILDER_H
#define UNIVERSITYAPLICATIONDEFINITIONBUILDER_H

#include "WorkflowDefinitionBuilder.h"
#include "DataRuleRouting.h"
#include "RoleBasedRouting.h"
/**
 * @class UniversityApplicationDefinitionBuilder
 * @brief Builds the university admissions workflow definition.
 */
class UniversityApplicationDefinitionBuilder : public WorkflowDefinitionBuilder
{
public:
	/**
	 * @brief Creates a university admissions builder with supplied metadata.
	 * @param id Workflow definition identifier.
	 * @param name Workflow definition name.
	 * @param version Workflow definition version.
	 */
	UniversityApplicationDefinitionBuilder(string id, string name, int version);
	UniversityApplicationDefinitionBuilder() = delete;
	UniversityApplicationDefinitionBuilder(const UniversityApplicationDefinitionBuilder &other) = delete;
	UniversityApplicationDefinitionBuilder &operator=(const UniversityApplicationDefinitionBuilder &other) = delete;

	void buildDefinition() override;
};

#endif
