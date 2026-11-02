#ifndef WORKFLOWDIRECTOR_H
#define WORKFLOWDIRECTOR_H

#include "WorkflowDefinitionBuilder.h"
#include <string>
using namespace std;

/**
 * @class WorkflowDirector
 * @brief Runs the same build for every concrete workflow builder.
 */
class WorkflowDirector
{

public:
	/**
	 * @brief Run structure, dependencies, services, routing and extras, then take the product.
	 * @param b file builder, or any other concrete builder
	 * @return WorkflowDefinition* finished definition, or nullptr when the id is missing or already used
	 */
	WorkflowDefinition* construct(WorkflowDefinitionBuilder* b);
};

#endif
