#ifndef REJECTACTION_H
#define REJECTACTION_H

/**
 * @file RejectAction.h
 * @author Katlego Sentsho
 * @brief
 * @version 0.1
 * @date 2026-11-01
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "WorkItemAction.h"

/**
 * @brief The RejectAction class represents an action to reject a work item in a workflow.
 *
 */
class RejectAction : public WorkItemAction
{

private:
	std::string reason;

protected:
	/** @brief Executes the reject action on the specified work item.
	 * @param item The work item to reject.
	 * @return true if the action was successful, false otherwise.
	 */
	bool doExecute(WorkItem &item);

public:
	/** @brief Constructs a new RejectAction instance.
	 * @param instance The workflow instance to which the action belongs.
	 * @param itemId The ID of the work item to reject.
	 * @param actor The participant who is rejecting the work item.
	 * @param reason The reason for rejecting the work item.
	 */
	RejectAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor, const std::string &reason);
	/** @brief Returns a description of the reject action.
	 * @return A string describing the reject action.
	 */
	std::string describe() const;
};

#endif
