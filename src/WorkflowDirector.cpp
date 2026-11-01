#include "WorkflowDirector.h"

WorkflowDefinition* WorkflowDirector::construct(WorkflowDefinitionBuilder* b)
{
	if (b == nullptr)
		return nullptr;
	b->buildStructure();
	b->buildDependencies();
	b->buildServices();
	b->buildRouting();
	b->buildExtras();
	return b->getResult();
}
