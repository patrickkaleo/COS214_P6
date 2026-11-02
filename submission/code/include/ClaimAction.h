/**
 * @file ClaimAction.h
 * @author Katlego Sentsho
 * @brief
 * @version 0.1
 * @date 2026-11-01
 *
 * @copyright Copyright (c) 2026
 *
 */
#ifndef CLAIMACTION_H
#define CLAIMACTION_H

#include "WorkItemAction.h"

/**
 * @brief The ClaimAction class represents an action to claim a work item in a workflow.
 *
 * This class inherits from WorkItemAction and implements the functionality to claim a work item.
 */
class ClaimAction : public WorkItemAction
{
protected:
	/** @brief Executes the claim action on the specified work item.
	 * @param item The work item to claim.
	 * @return true if the action was successful, false otherwise.
	 */
	bool doExecute(WorkItem &item);

public:
	/** @brief Constructs a new ClaimAction instance.
	 * @param instance The workflow instance to which the action belongs.
	 * @param itemId The ID of the work item to claim.
	 * @param actor The participant who is claiming the work item.
	 */
	ClaimAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor);

	std::string describe() const;
};

#endif
