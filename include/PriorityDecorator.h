/**
 * @file PriorityDecorator.h
 * @brief Concrete Decorator that adds a priority level to any WorkItem.
 * @details When the item becomes available it is flagged as urgent before the
 *          call is forwarded to the inner item.
 * @version 1.0
 */

#ifndef PRIORITYDECORATOR_H
#define PRIORITYDECORATOR_H

#include "WorkItemDecorator.h"

/**
 * @class PriorityDecorator
 * @brief Adds priority handling around the standard WorkItem lifecycle.
 *
 * Use (from the Builder):
 * @code
 *   WorkItem* item = new Task("t2", "Bursary Payment");
 *   item = new PriorityDecorator(item, 5);   // wrap
 *   parent->add(item);
 * @endcode
 */
class PriorityDecorator : public WorkItemDecorator {
public:
    /**
     * @brief Constructs a priority decorator.
     * @param inner WorkItem to wrap (ownership transferred).
     * @param level Priority level (higher = more urgent).
     */
    PriorityDecorator(WorkItem* inner, int level);

    /** @brief Default destructor (inner is deleted by base). */
    ~PriorityDecorator() override = default;

    /**
     * @return Configured priority level.
     */
    int getPriority() const;

    /**
     * @brief Flag the item as urgent, then forward onAvailable to the inner item.
     */
    void onAvailable() override;

    /**
     * @brief Deep-copy: clone the inner item and wrap it in a new PriorityDecorator.
     * @return Newly allocated PriorityDecorator*; caller owns it.
     */
    WorkItem* clone() const override;

private:
    int level_;   /**< Priority level. */
};

#endif // PRIORITYDECORATOR_H
