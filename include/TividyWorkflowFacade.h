#ifndef TIVIDYWORKFLOWFACADE_H
#define TIVIDYWORKFLOWFACADE_H

class TividyWorkflowFacade {

private:
	WorkflowEngine* engine;
	WorkflowDirector* director;

public:
	string registerDefinition(WorkflowDefinitionBuilder& builder);

	void addParticipant(string id, string name, vector<Role> roles);

	void addService(ExternalService* service);

	void addListener(WorkflowEventListener* listener);

	string startInstance(string definitionId, InstanceData data);

	bool claim(string instanceId, string itemId, string actorId);

	bool start(string instanceId, string itemId, string actorId);

	bool complete(string instanceId, string itemId, string actorId, string outcome);

	bool reject(string instanceId, string itemId, string actorId, string reason);

	bool delegate(string instanceId, string itemId, string actorId, string toId);

	bool cancel(string instanceId, string itemId, string actorId);

	bool undoLast();

	void checkDeadlines(time_t now);

	string getStatus(string instanceId);
};

#endif
