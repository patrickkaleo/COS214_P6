#ifndef COMPLETEACTION_H
#define COMPLETEACTION_H

/**
 * @file CompleteAction.h
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
 * @brief The CompleteAction class represents an action to complete a work item in a workflow.
 *
 */

class CompleteAction : public WorkItemAction
{

private:
	std::string outcome;

protected:
	/** @brief Generates a snapshot label for the specified work item.
	 * @param item The work item for which to generate the snapshot label.
	 * @return A string representing the snapshot label.
	 */
	std::string snapshotLabel(const WorkItem &item) const override;

	/**
	 * @brief Executes the complete action on the specified work item.
	 * @param item The work item to complete.
	 * @return true if the action was successful, false otherwise.
	 */
	bool doExecute(WorkItem &item);

public:
	/**
	 * @brief Construct a new CompleteAction object.
	 * @param instance The workflow instance to which the action belongs.
	 * @param itemId The ID of the work item to complete.
	 * @param actor The participant who is completing the work item.
	 * @param outcome The outcome of the completion.
	 */
	CompleteAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor, const std::string &outcome);

	/** @brief Returns a description of the complete action.
	 * @return A string describing the complete action.
	 */
	std::string describe() const;
};

#endif
