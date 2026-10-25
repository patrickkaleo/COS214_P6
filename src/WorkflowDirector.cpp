#include "WorkflowDirector.h"

WorkflowDefinition* WorkflowDirector::construct(WorkflowDefinitionBuilder& b)
{
	b.buildStructure();
	b.buildDependencies();
	b.buildRouting();
	b.buildIntegrations();
	b.buildExtras();
	return b.getResult();
}
