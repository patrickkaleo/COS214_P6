#ifndef WORKFLOWDIRECTOR_H
#define WORKFLOWDIRECTOR_H

#include "WorkflowDefinitionBuilder.h"

class WorkflowDirector
{

public:
	/**
	 * @brief use workflow definition builder to build a workflow defitinition
	 * @param WorkflowDefinitionBuildr&
	 */
	WorkflowDefinition* construct(WorkflowDefinitionBuilder& b);
};

#endif
