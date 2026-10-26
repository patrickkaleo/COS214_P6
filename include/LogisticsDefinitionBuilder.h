#ifndef LOGISTICSDEFINITIONBUILDER_H
#define LOGISTICSDEFINITIONBUILDER_H

#include "WorkflowDefinitionBuilder.h"
#include "DataRuleRouting.h"
#include "RoleBasedRouting.h"

/**
 * @class LogisticsDefinitionBuilder
 * @brief Builds the shipment-exception workflow definition.
 */
class LogisticsDefinitionBuilder : public WorkflowDefinitionBuilder
{
public:
	/**
	 * @brief Creates the logistics workflow builder with supplied metadata.
	 * @param id Workflow definition identifier.
	 * @param name Workflow definition name.
	 * @param version Workflow definition version.
	 */
	LogisticsDefinitionBuilder(string id, string name, int version);
	LogisticsDefinitionBuilder() = delete;
	LogisticsDefinitionBuilder(const LogisticsDefinitionBuilder& other) = delete;
	LogisticsDefinitionBuilder& operator=(const LogisticsDefinitionBuilder& other) = delete;

	void buildDefinition() override;
};

#endif
