/**
 * @file EventDispatcher.cpp
 * @brief Implementation of the EventDispatcher Singleton Subject class.
 * @author Enock Nyamweya
 * @version 2.0
 */

#include "EventDispatcher.h"

EventDispatcher::EventDispatcher()
    : listeners() {
}

EventDispatcher::~EventDispatcher() {
    clearListeners();
}

EventDispatcher& EventDispatcher::getInstance() {
    // C++11 thread-safe Meyers' Singleton
    static EventDispatcher instance;
    return instance;
}

void EventDispatcher::attachListener(std::shared_ptr<WorkflowEventListener> listener) {
    if (!listener) {
        return;
    }
    // Prevent duplicate registrations
    for (const auto& registration : listeners) {
        if (registration->listener.get() == listener.get()) {
            if (!registration->owning) {
                registration->listener = listener;
                registration->owning = true;
            }
            return;
        }
    }
    listeners.push_back(std::make_shared<Registration>(listener, true));
}

void EventDispatcher::attachListener(WorkflowEventListener* listener) {
    if (!listener) {
        return;
    }
    // Check if raw listener pointer is already registered
    for (const auto& registration : listeners) {
        if (registration->listener.get() == listener) {
            return;
        }
    }
    // Wrap with a non-owning custom no-op deleter so stack-allocated listeners
    // or externally managed observers are never deleted by shared_ptr
    std::shared_ptr<WorkflowEventListener> nonOwning(listener, [](WorkflowEventListener*) {});
    listeners.push_back(std::make_shared<Registration>(nonOwning, false));
}

void EventDispatcher::detachListener(std::shared_ptr<WorkflowEventListener> listener) {
    detachListener(listener.get());
}

void EventDispatcher::detachListener(WorkflowEventListener* listener) {
    if (!listener) {
        return;
    }
    for (auto it = listeners.begin(); it != listeners.end(); ++it) {
        if ((*it)->listener.get() == listener) {
            // Finish changing the collection before releasing an owned observer;
            // its destructor may itself unsubscribe or publish an event.
            const auto removed = *it;
            removed->active = false;
            listeners.erase(it);
            return;
        }
    }
}

void EventDispatcher::notifyListeners(const WorkflowEvent& event) {
    const auto pending = listeners;
    for (const auto& registration : pending) {
        if (registration->active) {
            // Pin ownership even if the callback removes its own registration.
            const auto listener = registration->listener;
            listener->onEvent(event);
        }
    }
}

size_t EventDispatcher::getListenerCount() const {
    return listeners.size();
}

void EventDispatcher::clearListeners() {
    std::vector<std::shared_ptr<Registration>> removed;
    removed.swap(listeners);
    for (const auto& registration : removed) {
        registration->active = false;
    }
}
