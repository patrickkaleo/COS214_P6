/**
 * @file EventDispatcher.cpp
 * @brief Implementation of the EventDispatcher Subject class for the Observer pattern.
 * @version 1.0
 */

#include "EventDispatcher.h"

EventDispatcher::EventDispatcher() {
    // Empty initialization of listeners vector
}

EventDispatcher::~EventDispatcher() {
    listeners.clear();
}

void EventDispatcher::attachListener(WorkflowEventListener* listener) {
    if (!listener) {
        return;
    }
    // Prevent duplicate registrations
    auto it = std::find(listeners.begin(), listeners.end(), listener);
    if (it == listeners.end()) {
        listeners.push_back(listener);
    }
}

void EventDispatcher::detachListener(WorkflowEventListener* listener) {
    if (!listener) {
        return;
    }
    auto it = std::find(listeners.begin(), listeners.end(), listener);
    if (it != listeners.end()) {
        listeners.erase(it);
    }
}

void EventDispatcher::notifyListeners(const WorkflowEvent& event) {
    for (WorkflowEventListener* listener : listeners) {
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
