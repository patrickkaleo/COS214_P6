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
    listeners.clear();
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
    for (const auto& sp : listeners) {
        if (sp == listener || sp.get() == listener.get()) {
            return;
        }
    }
    listeners.push_back(listener);
}

void EventDispatcher::attachListener(WorkflowEventListener* listener) {
    if (!listener) {
        return;
    }
    // Check if raw listener pointer is already registered
    for (const auto& sp : listeners) {
        if (sp.get() == listener) {
            return;
        }
    }
    // Wrap with a non-owning custom no-op deleter so stack-allocated listeners
    // or externally managed observers are never deleted by shared_ptr
    listeners.push_back(std::shared_ptr<WorkflowEventListener>(listener, [](WorkflowEventListener*) {}));
}

void EventDispatcher::detachListener(std::shared_ptr<WorkflowEventListener> listener) {
    if (!listener) {
        return;
    }
    for (auto it = listeners.begin(); it != listeners.end(); ++it) {
        if (*it == listener || it->get() == listener.get()) {
            listeners.erase(it);
            return;
        }
    }
}

void EventDispatcher::detachListener(WorkflowEventListener* listener) {
    if (!listener) {
        return;
    }
    for (auto it = listeners.begin(); it != listeners.end(); ++it) {
        if (it->get() == listener) {
            listeners.erase(it);
            return;
        }
    }
}

void EventDispatcher::notifyListeners(const WorkflowEvent& event) {
    for (const auto& listener : listeners) {
        if (listener) {
            listener->onEvent(event);
        }
    }
}

size_t EventDispatcher::getListenerCount() const {
    return listeners.size();
}

void EventDispatcher::clearListeners() {
    listeners.clear();
}
