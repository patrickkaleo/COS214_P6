#include "WorkflowDirector.h"

WorkflowDefinition* WorkflowDirector::construct(WorkflowDefinitionBuilder& b)
{
	b.buildDefinition();
	return b.getResult();
}
