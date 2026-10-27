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
	 * @brief Tell the builder to build the definition, then take the product.
	 * @param b concrete builder for one scenario, or a file builder
	 * @return WorkflowDefinition* finished definition, or nullptr when the id is missing or already used
	 */
	WorkflowDefinition* construct(WorkflowDefinitionBuilder* b);
};

#endif
