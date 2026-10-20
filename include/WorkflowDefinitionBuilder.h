#ifndef WORKFLOWDEFINITIONBUILDER_H
#define WORKFLOWDEFINITIONBUILDER_H

class WorkflowDefinitionBuilder {

protected:
	WorkFlowDefinition definition;

public:
	void reset(string name, int version);

	virtual void buildstructure() = 0;

	virtual void buildDependencies() = 0;

	virtual void buildRouting() = 0;

	virtual void buildIntergrations() = 0;

	WorkflowDefinition* getResult();

protected:
	void addSubProcess(string id, string name, string parentId);

	void addTask(string id, string name, string parentId);

	void addDependency(string fromId, string toId, string outcome);

	void addSequence(vector<string> ids);

	void setRouting(string itemId, RoutingStrategy* s);

	void setService(string itemId, string serviceName);

	void markCheckpoint(stringId item);

	void addPriority(string itemId, int level);

	void addAudit(string itemId);
};

#endif
