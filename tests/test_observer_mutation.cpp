#include "doctest.h"
#include "EventDispatcher.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace {
class CallbackListener : public WorkflowEventListener {
public:
    std::function<void(const WorkflowEvent&)> callback;
    std::function<void()> onDestroy;

    explicit CallbackListener(std::function<void(const WorkflowEvent&)> action,
                              std::function<void()> destroyed = std::function<void()>())
        : callback(action), onDestroy(destroyed) {}

    ~CallbackListener() override {
        if (onDestroy) onDestroy();
    }

    void onEvent(const WorkflowEvent& event) override { callback(event); }
};

struct DispatchReset {
    EventDispatcher& dispatcher;
    DispatchReset() : dispatcher(EventDispatcher::getInstance()) { dispatcher.clearListeners(); }
    ~DispatchReset() { dispatcher.clearListeners(); }
};

WorkflowEvent mutationEvent(const std::string& name = "outer") {
    return WorkflowEvent(EventType::STARTED, "mutation-instance", "task", name);
}
}

TEST_SUITE("Observer mutation and lifetime") {
    TEST_CASE("Listeners added during delivery begin with the next dispatch") {
        bool added = false;
        int existingCalls = 0;
        int newCalls = 0;
        DispatchReset reset;
        auto& dispatcher = reset.dispatcher;
        auto adder = std::make_shared<CallbackListener>([&](const WorkflowEvent&) {
            if (added) return;
            added = true;
            // Enough registrations to force reallocation of the live collection.
            for (int i = 0; i < 128; ++i) {
                dispatcher.attachListener(std::make_shared<CallbackListener>(
                    [&](const WorkflowEvent&) { ++newCalls; }));
            }
        });
        dispatcher.attachListener(adder);
        dispatcher.attachListener(std::make_shared<CallbackListener>(
            [&](const WorkflowEvent&) { ++existingCalls; }));

        dispatcher.notifyListeners(mutationEvent());
        CHECK(existingCalls == 1);
        CHECK(newCalls == 0);
        CHECK(dispatcher.getListenerCount() == 130);
        dispatcher.notifyListeners(mutationEvent());
        CHECK(existingCalls == 2);
        CHECK(newCalls == 128);
    }

    TEST_CASE("Self-detachment retains the sole shared owner until callback returns") {
        bool destroyed = false;
        bool returnedFromDetach = false;
        CallbackListener* raw = nullptr;
        DispatchReset reset;
        auto& dispatcher = reset.dispatcher;
        auto listener = std::make_shared<CallbackListener>([&](const WorkflowEvent&) {
            dispatcher.detachListener(raw);
            CHECK_FALSE(destroyed);
            CHECK(dispatcher.getListenerCount() == 0);
            returnedFromDetach = true;
        }, [&]() { destroyed = true; });
        raw = listener.get();
        dispatcher.attachListener(listener);
        listener.reset();

        dispatcher.notifyListeners(mutationEvent());
        CHECK(returnedFromDetach);
        CHECK(destroyed);
    }

    TEST_CASE("Detaching and destroying a pending raw listener cancels its delivery") {
        int victimCalls = 0;
        bool victimDestroyed = false;
        std::unique_ptr<CallbackListener> victim(new CallbackListener(
            [&](const WorkflowEvent&) { ++victimCalls; }, [&]() { victimDestroyed = true; }));
        DispatchReset reset;
        auto& dispatcher = reset.dispatcher;
        dispatcher.attachListener(std::make_shared<CallbackListener>([&](const WorkflowEvent&) {
            dispatcher.detachListener(victim.get());
            victim.reset();
            CHECK(victimDestroyed);
        }));
        dispatcher.attachListener(victim.get());

        dispatcher.notifyListeners(mutationEvent());
        CHECK(victimCalls == 0);
        CHECK(victimDestroyed);
        CHECK(dispatcher.getListenerCount() == 1);
    }

    TEST_CASE("Clearing listeners cancels pending callbacks and nested delivery") {
        int clearerCalls = 0;
        int pendingCalls = 0;
        bool clearerDestroyed = false;
        DispatchReset reset;
        auto& dispatcher = reset.dispatcher;
        dispatcher.attachListener(std::make_shared<CallbackListener>([&](const WorkflowEvent&) {
            ++clearerCalls;
            dispatcher.clearListeners();
            CHECK_FALSE(clearerDestroyed);
            dispatcher.notifyListeners(mutationEvent("nested"));
        }, [&]() { clearerDestroyed = true; }));
        dispatcher.attachListener(std::make_shared<CallbackListener>(
            [&](const WorkflowEvent&) { ++pendingCalls; }));

        dispatcher.notifyListeners(mutationEvent());
        CHECK(clearerCalls == 1);
        CHECK(pendingCalls == 0);
        CHECK(clearerDestroyed);
        CHECK(dispatcher.getListenerCount() == 0);
    }

    TEST_CASE("Detach and reattach creates a new registration for the next dispatch") {
        bool replaced = false;
        int pendingCalls = 0;
        CallbackListener pending([&](const WorkflowEvent&) { ++pendingCalls; });
        DispatchReset reset;
        auto& dispatcher = reset.dispatcher;
        dispatcher.attachListener(std::make_shared<CallbackListener>([&](const WorkflowEvent&) {
            if (replaced) return;
            replaced = true;
            dispatcher.detachListener(&pending);
            dispatcher.attachListener(&pending);
        }));
        dispatcher.attachListener(&pending);

        dispatcher.notifyListeners(mutationEvent());
        CHECK(pendingCalls == 0);
        CHECK(dispatcher.getListenerCount() == 2);
        dispatcher.notifyListeners(mutationEvent());
        CHECK(pendingCalls == 1);
    }

    TEST_CASE("Nested dispatch observes current registrations without reviving detached ones") {
        std::vector<std::string> calls;
        CallbackListener removed([&](const WorkflowEvent& e) { calls.push_back("removed-" + e.itemName); });
        CallbackListener added([&](const WorkflowEvent& e) { calls.push_back("added-" + e.itemName); });
        DispatchReset reset;
        auto& dispatcher = reset.dispatcher;
        dispatcher.attachListener(std::make_shared<CallbackListener>([&](const WorkflowEvent& e) {
            calls.push_back("first-" + e.itemName);
            if (e.itemName == "outer") {
                dispatcher.detachListener(&removed);
                dispatcher.attachListener(&added);
                dispatcher.notifyListeners(mutationEvent("nested"));
            }
        }));
        dispatcher.attachListener(&removed);

        dispatcher.notifyListeners(mutationEvent());
        const std::vector<std::string> expected = {"first-outer", "first-nested", "added-nested"};
        CHECK(calls == expected);
        CHECK(dispatcher.getListenerCount() == 2);
    }

    TEST_CASE("Shared registration upgrades raw ownership without duplicate delivery") {
        int calls = 0;
        bool destroyed = false;
        DispatchReset reset;
        auto& dispatcher = reset.dispatcher;
        auto listener = std::make_shared<CallbackListener>(
            [&](const WorkflowEvent&) { ++calls; }, [&]() { destroyed = true; });
        CallbackListener* raw = listener.get();
        std::weak_ptr<CallbackListener> weak = listener;
        dispatcher.attachListener(raw);
        dispatcher.attachListener(listener);
        dispatcher.attachListener(raw);
        dispatcher.attachListener(listener);
        listener.reset();

        CHECK_FALSE(weak.expired());
        CHECK(dispatcher.getListenerCount() == 1);
        dispatcher.notifyListeners(mutationEvent());
        CHECK(calls == 1);
        dispatcher.detachListener(raw);
        CHECK(weak.expired());
        CHECK(destroyed);
    }

    TEST_CASE("Clearing ownership permits observer destructors to reenter safely") {
        int callbacks = 0;
        int destroyed = 0;
        CallbackListener* pending = nullptr;
        DispatchReset reset;
        auto& dispatcher = reset.dispatcher;
        dispatcher.attachListener(std::make_shared<CallbackListener>(
            [&](const WorkflowEvent&) { ++callbacks; }, [&]() {
                ++destroyed;
                CHECK(dispatcher.getListenerCount() == 0);
                dispatcher.detachListener(pending);
                dispatcher.notifyListeners(mutationEvent("destructor"));
            }));
        auto listener = std::make_shared<CallbackListener>(
            [&](const WorkflowEvent&) { ++callbacks; }, [&]() { ++destroyed; });
        pending = listener.get();
        dispatcher.attachListener(listener);
        listener.reset();

        dispatcher.clearListeners();
        CHECK(destroyed == 2);
        CHECK(callbacks == 0);
        CHECK(dispatcher.getListenerCount() == 0);
    }
}
