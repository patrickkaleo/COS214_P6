/**
 * @file TividyWorkflowFacade.h
 * @brief This file contains the declaration of the TividyWorkflowFacade class, which serves as a facade for the workflow engine and director.
 * @author Katlego Sentsho
 * @date 2024-06-10
 */
#ifndef TIVIDYWORKFLOWFACADE_H
#define TIVIDYWORKFLOWFACADE_H

#include "WorkflowEngine.h"
#include "WorkflowDirector.h"
#include "InstanceData.h"

class Participant;
class ExternalService;
class WorkflowEventListener;
class WorkflowDefinitionBuilder;

class TividyWorkflowFacade
{

private:
	WorkflowEngine *engine;
	WorkflowDirector *director;

public:
	/**
	 * @brief Constructs a new TividyWorkflowFacade object.
	 *
	 * Initializes the workflow engine and director.
	 */
	TividyWorkflowFacade();

	/**
	 * @brief Destroys the TividyWorkflowFacade object.
	 *
	 * Cleans up the workflow engine and director.
	 */
	~TividyWorkflowFacade();

	/**
	 * @brief Registers a new workflow definition.
	 *
	 * @param builder The workflow definition builder.
	 * @return The ID of the registered definition.
	 */
	std::string registerDefinition(WorkflowDefinitionBuilder &builder);

	/**
	 * @brief Adds a participant to the directory.
	 *
	 * @param id The ID of the participant.
	 * @param name The name of the participant.
	 * @param roles The roles of the participant.
	 */
	void addParticipant(std::string id, std::string name, std::vector<std::string> roles);

	/**
	 * @brief Registers an external system adapter.
	 *
	 * @param service The external service to register.
	 */
	void addService(ExternalService *service);

	/**
	 * @brief Registers an event listener.
	 *
	 * @param listener The event listener to register.
	 */
	void addListener(WorkflowEventListener *listener);

	/**
	 * @brief Starts a new workflow instance.
	 *
	 * @param definitionId The ID of the workflow definition.
	 * @param data The instance data.
	 * @return The ID of the started instance.
	 */
	std::string startInstance(std::string definitionId, InstanceData data);

	/**
	 * @brief Claims a work item.
	 *
	 * @param instanceId The ID of the workflow instance.
	 * @param itemId The ID of the work item.
	 * @param actorId The ID of the actor claiming the item.
	 * @return True if the claim is successful, false otherwise.
	 */
	bool claim(std::string instanceId, std::string itemId, std::string actorId);

	/**
	 * @brief Starts work on an item.
	 *
	 * @param instanceId The ID of the workflow instance.
	 * @param itemId The ID of the work item.
	 * @param actorId The ID of the actor starting the work.
	 * @return True if the start is successful, false otherwise.
	 */
	bool start(std::string instanceId, std::string itemId, std::string actorId);

	/**
	 * @brief Completes a work item.
	 *
	 * @param instanceId The ID of the workflow instance.
	 * @param itemId The ID of the work item.
	 * @param actorId The ID of the actor completing the item.
	 * @param outcome The outcome of the completion.
	 * @return True if the completion is successful, false otherwise.
	 */
	bool complete(std::string instanceId, std::string itemId, std::string actorId, std::string outcome);

	/**
	 * @brief Rejects a work item.
	 *
	 * @param instanceId The ID of the workflow instance.
	 * @param itemId The ID of the work item.
	 * @param actorId The ID of the actor rejecting the item.
	 * @param reason The reason for rejection.
	 * @return True if the rejection is successful, false otherwise.
	 */
	bool reject(std::string instanceId, std::string itemId, std::string actorId, std::string reason);

	/**
	 * @brief Delegates a work item.
	 *
	 * @param instanceId The ID of the workflow instance.
	 * @param itemId The ID of the work item.
	 * @param actorId The ID of the actor delegating the item.
	 * @param toId The ID of the actor to whom the item is being delegated.
	 * @return True if the delegation is successful, false otherwise.
	 */
	bool delegate(std::string instanceId, std::string itemId, std::string actorId, std::string toId);

	/**
	 * @brief Cancels a work item.
	 *
	 * @param instanceId The ID of the workflow instance.
	 * @param itemId The ID of the work item.
	 * @param actorId The ID of the actor canceling the item.
	 * @return True if the cancellation is successful, false otherwise.
	 */
	bool cancel(std::string instanceId, std::string itemId, std::string actorId);

	/**
	 * @brief Undoes the last executed action.
	 *
	 * @return True if the undo is successful, false otherwise.
	 */
	bool undoLast();

	/**
	 * @brief Checks for any expired deadlines.
	 *
	 * @param now The current time.
	 */
	void checkDeadlines(time_t now);

	/**
	 * @brief Retrieves the current status of a workflow instance.
	 *
	 * @param instanceId The ID of the workflow instance.
	 * @return A string representing the current status of the instance.
	 */
	std::string getStatus(std::string instanceId);
};

#endif
