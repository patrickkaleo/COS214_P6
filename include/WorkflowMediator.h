#ifndef WORKFLOWMEDIATOR_H
#define WORKFLOWMEDIATOR_H

/**
 * @file WorkflowMediator.h
 * @brief Abstract Mediator coordinating the work items of one workflow instance.
 * @author Angelo Anthony
 * @version 1.0
 */

class WorkItem;
class SubProcess;

/**
 * @class WorkflowMediator
 * @brief Mediator of the GoF Mediator (Behavioural) pattern.
 *
 * Finishing one work item can make others available, and branches can depend on
 * each other. Work items (the Colleagues) never reference each other. Instead each
 * one tells its mediator when it finishes, and the mediator decides what happens next.
 *
 * @see DependencyMediator
 */
class WorkflowMediator {
public:
    virtual ~WorkflowMediator() = default;

    /**
     * @brief Adds an item to the set this mediator coordinates and links it back.
     * @param item Item to coordinate (not owned).
     */
    virtual void registerItem(WorkItem* item) = 0;

    /**
     * @brief Called by a work item after it completes, is rejected or is cancelled.
     * @param item The item that finished.
     */
    virtual void itemChanged(WorkItem& item) = 0;

    /**
     * @brief Called by a sub-process when it starts, to make its entry children available.
     * @param subProcess The sub-process that has just started.
     */
    virtual void openChildren(SubProcess& subProcess) = 0;
};

#endif // WORKFLOWMEDIATOR_H
