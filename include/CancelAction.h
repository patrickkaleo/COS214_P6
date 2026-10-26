/**
 * @file CancelAction.h
 * @brief Header file for the CancelAction class
 *
 * This file defines the CancelAction class, which is responsible for executing
 * the cancel action on a given work item. It inherits from the WorkItemAction
 * base class and provides implementations for executing the action and describing it.
 */
#ifndef CANCELACTION_H
#define CANCELACTION_H

#include "WorkItemAction.h"

/**
 * @brief The CancelAction class is responsible for executing the cancel action on a work item.
 * @author Katlego Sentsho
 *
 */

class CancelAction : WorkItemAction
{

protected:
	/**
	 * @brief Executes the cancel action on the given work item
	 *
	 * @param Item The work item to cancel
	 * @return true if the action was successful, false otherwise
	 */
	bool doExecute(WorkItem &Item);

public:
	/**
	 * @brief Describes the cancel action
	 *
	 * @return A string description of the action
	 */
	std::string describe();
};

#endif
