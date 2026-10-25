#ifndef WORKITEMITERATOR_H
#define WORKITEMITERATOR_H

/**
 * @file WorkItemIterator.h
 * @brief Iterator interface over a work-item tree.
 * @author Angelo Anthony
 * @version 1.0
 */

#include "Iterator.h"

class WorkItem;

/**
 * @class WorkItemIterator
 * @brief Iterator over the WorkItem Composite (Task leaves and SubProcess branches).
 *
 * Returned by SubProcess::createIterator(). Used to register a tree with its
 * mediator, to find items by id and to list current work in the interface.
 */
class WorkItemIterator : public Iterator<WorkItem> {
};

#endif // WORKITEMITERATOR_H
