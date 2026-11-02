#ifndef STARTACTION_H
#define STARTACTION_H

/**
 * @file StartAction.h
 * @author Katlego Sentsho
 * @brief
 * @version 0.1
 * @date 2026-11-01
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "WorkItemAction.h"

class StartAction : public WorkItemAction
{
protected:
    /** @brief Executes the start action on the specified work item.
     * @param item The work item to start.
     * @return true if the action was successful, false otherwise.
     */
    bool doExecute(WorkItem &item) override;

public:
    /** @brief Constructs a new StartAction instance.
     * @param instance The workflow instance to which the action belongs.
     * @param itemId The ID of the work item to start.
     * @param actor The participant who is starting the work item.
     */
    StartAction(WorkflowInstance *instance,
                const std::string &itemId,
                Participant *actor);

    /** @brief Returns a description of the start action.
     * @return A string describing the start action.
     */
    std::string describe() const override;
};

#endif
