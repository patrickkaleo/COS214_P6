/**
 * @file WorkItemDecorator.h
 * @brief Abstract Decorator for WorkItem (Structural pattern).
 * @details Owns an inner WorkItem and forwards every operation to it.
 *          Concrete decorators add behaviour before or after the forward.
 * @version 1.0
 */

#ifndef WORKITEMDECORATOR_H
#define WORKITEMDECORATOR_H

#include "WorkItem.h"
#include <string>

class Participant;
class WorkflowEventListener;
class WorkflowEvent;

/**
 * @class WorkItemDecorator
 * @brief Abstract decorator. Adopts the id/name of the wrapped item so the
 *        rest of the engine still sees the original identity.
 *
 * Decorators can be stacked (e.g. PriorityDecorator → AuditDecorator → Task)
 */
class WorkItemDecorator : public WorkItem {
public:
    /**
     * @brief Constructs a decorator around the given WorkItem.
     * @param inner WorkItem being decorated (ownership is transferred).
     *              The decorator adopts inner's id and name.
     */
    explicit WorkItemDecorator(WorkItem* inner);

    /**
     * @brief Destructor. Deletes the owned inner WorkItem.
     */
    ~WorkItemDecorator() override;

    // forward lifecycle

    /** @brief Forward makeAvailable to the inner item. */
    bool makeAvailable() override;

    /** @brief Forward assign to the inner item. */
    bool assign(Participant* participant) override;

    /** @brief Forward start to the inner item. */
    bool start() override;

    /** @brief Forward complete to the inner item. */
    bool complete(const std::string& outcome) override;

    /** @brief Forward reject to the inner item. */
    bool reject(const std::string& reason) override;

    /** @brief Forward cancel to the inner item. */
    bool cancel() override;

    /** @brief Forward escalate to the inner item. */
    bool escalate() override;

    /**
     * @brief Forward onAvailable to the inner item.
     *        Concrete decorators add behaviour then call this.
     */
    void onAvailable() override;

    /**
     * @brief Pure virtual clone – each concrete decorator decides how to copy.
     * @return Newly allocated decorator wrapping a clone of the inner item.
     */
    WorkItem* clone() const override = 0;

    /**
     * @return Pointer to the wrapped WorkItem (owned by this decorator).
     */
    WorkItem* getInner() const;

    /**
     * @brief Give up ownership of the wrapped item without deleting it.
     * @return The inner item, or nullptr when this decorator holds nothing.
     */
    WorkItem* releaseInner();

protected:
    WorkItem* inner_;   /**< Owned wrapped item. */
};

#endif // WORKITEMDECORATOR_H
