#ifndef EVENT_DISPATCHER_H
#define EVENT_DISPATCHER_H

/**
 * @file EventDispatcher.h
 * @brief Concrete Subject class managing observer registrations and event propagation.
 * @details Conforms to both the GoF Observer pattern (Subject role) and
 *          the GoF Singleton pattern (Creational category).
 * @author Enock Nyamweya
 * @version 2.0
 */

#include <vector>
#include <algorithm>
#include <memory>
#include "WorkflowEventListener.h"
#include "WorkflowEvent.h"

/**
 * @class EventDispatcher
 * @brief Singleton Subject in the GoF Observer pattern responsible for publishing workflow events to listeners.
 * @details Implements Meyers' Singleton (C++11 thread-safe, stack-allocated, zero memory leaks).
 *          Maintains a collection of registered WorkflowEventListener instances via std::shared_ptr,
 *          providing both modern shared pointer subscription and backward-compatible raw pointer
 *          overloads with non-owning custom deleters.
 */
class EventDispatcher {
private:
    struct Registration {
        std::shared_ptr<WorkflowEventListener> listener;
        bool owning;
        bool active;

        Registration(std::shared_ptr<WorkflowEventListener> value, bool ownsListener)
            : listener(value), owning(ownsListener), active(true) {}
    };

    // Records survive an in-progress dispatch so detachment can cancel pending delivery.
    std::vector<std::shared_ptr<Registration>> listeners;

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
     * @return EventDispatcher& Reference to the unique singleton instance.
     */
    static EventDispatcher& getInstance();

    /**
     * @brief Subscribes an observer managed via std::shared_ptr.
     * @param listener Shared pointer to the WorkflowEventListener to register.
     */
    void attachListener(std::shared_ptr<WorkflowEventListener> listener);

    /**
     * @brief Non-owning registration for externally managed or stack-allocated observers.
     * @details The caller must detach the observer before destroying it. Registering the
     *          same observer with shared ownership upgrades a non-owning registration.
     * @param listener Raw pointer to the WorkflowEventListener to register.
     */
    void attachListener(WorkflowEventListener* listener);

    /**
     * @brief Unsubscribes an observer managed via std::shared_ptr.
     * @param listener Shared pointer to the WorkflowEventListener to remove.
     */
    void detachListener(std::shared_ptr<WorkflowEventListener> listener);

    /**
     * @brief Backward-compatible overload for raw pointers.
     * @param listener Raw pointer to the WorkflowEventListener to remove.
     */
    void detachListener(WorkflowEventListener* listener);

    /**
     * @brief Broadcasts a workflow event to listeners registered when dispatch begins.
     * @details Callbacks may attach, detach, clear, or synchronously dispatch another event.
     *          Detached listeners awaiting delivery are skipped. New registrations receive
     *          subsequent dispatches, including nested dispatches, but not this event.
     *          Shared owners remain alive through their callback. Calls are single-threaded;
     *          thread-safe singleton initialization does not synchronize registration or dispatch.
     * @param event Const reference to the WorkflowEvent payload to deliver.
     */
    void notifyListeners(const WorkflowEvent& event);

    /**
     * @brief Returns the number of currently active listeners.
     * @return size_t Count of subscribed listeners.
     */
    size_t getListenerCount() const;

    /**
     * @brief Clears all registered listeners.
     */
    void clearListeners();
};

#endif // EVENT_DISPATCHER_H
