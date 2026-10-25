#ifndef UNIVERSITYAPLICATIONDEFINITIONBUILDER_H
#define UNIVERSITYAPLICATIONDEFINITIONBUILDER_H

#include "WorkflowDefinitionBuilder.h"

/**
 * @class UniversityApplicationDefinitionBuilder
 * @brief Generic Builder for creating workflow definitions that can be applied to different scenarios and set ups
 */
class UniversityApplicationDefinitionBuilder : public WorkflowDefinitionBuilder
{
public:
	UniversityApplicationDefinitionBuilder(string id);
	UniversityApplicationDefinitionBuilder() = delete;
	UniversityApplicationDefinitionBuilder(WorkflowDefinition& ) = delete;
	
	void buildStructure();
	void buildDependencies();
	void buildRouting();
	void buildIntegrations();
	void buildExtras();
};

#endif
