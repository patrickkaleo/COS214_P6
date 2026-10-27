#include "WorkflowDirector.h"
#include "FileWorkflowDefinitionBuilder.h"

WorkflowDefinition* WorkflowDirector::construct(WorkflowDefinitionBuilder* b)
{
	b->buildDefinition();
	return b->getResult();
}
