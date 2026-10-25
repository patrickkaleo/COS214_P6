#ifndef ITERATOR_H
#define ITERATOR_H

/**
 * @file Iterator.h
 * @brief Generic Iterator interface used to traverse TIVIDY's object structures.
 * @author Angelo Anthony
 * @version 1.0
 */

/**
 * @class Iterator
 * @brief Abstract Iterator of the GoF Iterator (Behavioural) pattern.
 *
 * TIVIDY has two recursive structures that clients need to walk without knowing
 * how they are stored: the work-item tree (Composite) and the approval chain
 * (Chain of Responsibility). Each provides a concrete iterator behind this one
 * interface, so traversal code is written once:
 * @code
 *   for (it->first(); !it->isDone(); it->next()) {
 *       use(it->current());
 *   }
 * @endcode
 *
 * @tparam T Element type returned by current().
 * @see DepthFirstWorkItemIterator, ApprovalChainIterator
 */
template <typename T>
class Iterator {
public:
    virtual ~Iterator() = default;

    /// Moves to the first element (restarts the traversal).
    virtual void first() = 0;

    /// Advances to the next element. Does nothing once the traversal is done.
    virtual void next() = 0;

    /// @return true once every element has been visited.
    virtual bool isDone() const = 0;

    /// @return The element at the current position, or nullptr when done.
    virtual T* current() const = 0;
};

#endif // ITERATOR_H
