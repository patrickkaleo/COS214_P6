/**
 * @file EventDispatcher.h
 * @brief Concrete Subject class managing observer registrations and event propagation.
 * @details Conforms to both the GoF Observer pattern (Subject role, Section 3.8) and
 *          the GoF Singleton pattern (Creational category, Chapter 28 of course notes).
 * @version 1.0
 */

#ifndef EVENT_DISPATCHER_H
#define EVENT_DISPATCHER_H

#include <vector>
#include <algorithm>
#include "WorkflowEventListener.h"
#include "WorkflowEvent.h"

/**
 * @class EventDispatcher
 * @brief Singleton Subject in the GoF Observer pattern responsible for publishing workflow events to listeners.
 * @details Implements Meyers' Singleton (C++11 thread-safe, stack-allocated, zero memory leaks).
 *          Maintains a collection of registered WorkflowEventListener instances and notifies them
 *          synchronously when lifecycle events occur.
 */
class EventDispatcher {
private:
    std::vector<WorkflowEventListener*> listeners; /**< Collection of registered observer pointers. */

    /**
     * @brief Private default constructor to enforce the Singleton pattern.
     */
    EventDispatcher();

public:
    /**
     * @brief Deleted copy constructor to prevent cloning the Singleton instance.
     */
    EventDispatcher(const EventDispatcher&) = delete;

    /**
     * @brief Deleted copy assignment operator to prevent assignment of the Singleton.
     */
    EventDispatcher& operator=(const EventDispatcher&) = delete;

    /**
     * @brief Virtual destructor ensuring clean cleanup of the dispatcher.
     */
    virtual ~EventDispatcher();

    /**
     * @brief Global static access point for the unique EventDispatcher Singleton instance.
     * @details Implemented as a Meyers' Singleton (C++11 guaranteed thread-safe static initialization).
     * @return EventDispatcher& Reference to the unique singleton instance.
     */
    static EventDispatcher& getInstance();

    /**
     * @brief Subscribes an observer to receive all workflow events.
     * @param listener Pointer to the WorkflowEventListener to register.
     * @details If the listener is nullptr or already registered, registration is ignored.
     */
    void attachListener(WorkflowEventListener* listener);

    /**
     * @brief Unsubscribes an observer from receiving further events.
     * @param listener Pointer to the WorkflowEventListener to remove.
     * @details If the listener is nullptr or not registered, the call has no effect.
     */
    void detachListener(WorkflowEventListener* listener);

    /**
     * @brief Broadcasts a workflow event to all currently registered listeners.
     * @param event Const reference to the WorkflowEvent payload to deliver.
     */
    void notifyListeners(const WorkflowEvent& event);

    /**
     * @brief Retrieves the number of currently attached listeners.
     * @return size_t Count of active listeners.
     */
    size_t getListenerCount() const;

    /**
     * @brief Removes all registered listeners from the dispatcher.
     */
    void clearListeners();
};

#endif // EVENT_DISPATCHER_H
