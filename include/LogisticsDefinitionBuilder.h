#ifndef LOGISTICSDEFINITIONBUILDER_H
#define LOGISTICSDEFINITIONBUILDER_H

#include "WorkflowDefinitionBuilder.h"
#include "RoleBasedRouting.h"
#include <vector>
using namespace std;
/**
 * @class LogisticsDefinitionBuilder
 * @brief Generic Builder for creating workflow definitions that can be applied to different scenarios and set ups
 */
class LogisticsDefinitionBuilder : public WorkflowDefinitionBuilder
{
public:
	LogisticsDefinitionBuilder(string id, string name, int version );
	LogisticsDefinitionBuilder() = delete ;
	LogisticsDefinitionBuilder(LogisticsDefinitionBuilder& other) = delete ;
	void buildStructure();
	void buildDependencies();
	void buildRouting();
	void buildIntegrations();
	void buildExtras();
};

#endif
