#include "WorkflowDirector.h"

WorkflowDefinition* WorkflowDirector::construct(WorkflowDefinitionBuilder* b)
{
	b->buildStructure();
	b->buildDependencies();
	b->buildServices();
	b->buildRouting();
	b->buildExtras();
	return b->getResult();
}
