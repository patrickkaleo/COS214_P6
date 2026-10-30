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
#include <string>
#include <vector>
#include <ctime>

class Participant;
class ExternalService;
class WorkflowEventListener;
class WorkflowDefinitionBuilder;

class TividyWorkflowFacade
{

private:
	WorkflowEngine engine;
	WorkflowDirector director;

public:
	/**
	 * @brief Constructs a new TividyWorkflowFacade object.
	 *
	 * Initializes the workflow engine and director.
	 */
	TividyWorkflowFacade() = default;

	/**
	 * @brief Destroys the TividyWorkflowFacade object.
	 *
	 * Cleans up the workflow engine and director.
	 */
	~TividyWorkflowFacade() = default;

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
	void addParticipant(const std::string &id, const std::string &name, const std::vector<std::string> &roles);

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
	std::string startInstance(const std::string &definitionId, const InstanceData &data);

	/**
	 * @brief Claims a work item.
	 *
	 * @param instanceId The ID of the workflow instance.
	 * @param itemId The ID of the work item.
	 * @param actor Pointer to the actor claiming the item.
	 * @return True if the claim is successful, false otherwise.
	 */
	bool claim(const std::string &instanceId, const std::string &itemId, Participant *actor);

	/**
	 * @brief Starts work on an item.
	 *
	 * @param instanceId The ID of the workflow instance.
	 * @param itemId The ID of the work item.
	 * @param actor Pointer to the actor starting the work.
	 * @return True if the start is successful, false otherwise.
	 */
	bool start(const std::string &instanceId, const std::string &itemId, Participant *actor);

	/**
	 * @brief Completes a work item.
	 *
	 * @param instanceId The ID of the workflow instance.
	 * @param itemId The ID of the work item.
	 * @param actor Pointer to the actor completing the item.
	 * @param outcome The outcome of the completion.
	 * @return True if the completion is successful, false otherwise.
	 */
	bool complete(const std::string &instanceId, const std::string &itemId, Participant *actor, const std::string &outcome);

	/**
	 * @brief Rejects a work item.
	 *
	 * @param instanceId The ID of the workflow instance.
	 * @param itemId The ID of the work item.
	 * @param actor Pointer to the actor rejecting the item.
	 * @param reason The reason for rejection.
	 * @return True if the rejection is successful, false otherwise.
	 */
	bool reject(const std::string &instanceId, const std::string &itemId, Participant *actor, const std::string &reason);

	/**
	 * @brief Delegates a work item.
	 *
	 * @param instanceId The ID of the workflow instance.
	 * @param itemId The ID of the work item.
	 * @param actor Pointer to the actor delegating the item.
	 * @param to Pointer to the target participant.
	 * @return True if the delegation is successful, false otherwise.
	 */
	bool delegate(const std::string &instanceId, const std::string &itemId, Participant *actor, Participant *to);

	/**
	 * @brief Cancels a work item.
	 *
	 * @param instanceId The ID of the workflow instance.
	 * @param itemId The ID of the work item.
	 * @param actor Pointer to the actor canceling the item.
	 * @return True if the cancellation is successful, false otherwise.
	 */
	bool cancel(const std::string &instanceId, const std::string &itemId, Participant *actor);

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
	std::string getStatus(const std::string &instanceId);

	std::string registerDefinitionFromFile(
			const std::string &filePath);
};

#endif
