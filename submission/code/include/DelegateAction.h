#ifndef DELEGATEACTION_H
#define DELEGATEACTION_H

/**
 * @file DelegateAction.h
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
 * @brief The DelegateAction class represents an action to delegate a work item in a workflow.
 *
 */
class DelegateAction : public WorkItemAction
{

private:
	Participant *to;

public:
	/** @brief Constructs a new DelegateAction instance.
	 * @param instance The workflow instance to which the action belongs.
	 * @param itemId The ID of the work item to delegate.
	 * @param actor The participant who is delegating the work item.
	 * @param to The participant to whom the work item is being delegated.
	 */
	DelegateAction(WorkflowInstance *instance, const std::string &itemId, Participant *actor, Participant *to);
	/** @brief Executes the delegate action on the specified work item.
	 * @param item The work item to delegate.
	 * @return true if the action was successful, false otherwise.
	 */
	bool doExecute(WorkItem &item);
	/** @brief Returns a description of the delegate action.
	 * @return A string describing the delegate action.
	 */
	std::string describe() const;
};

#endif
