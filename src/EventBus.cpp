#include "event/Events.hpp"

#include <stdexcept>

EventBus EventBus::instance{};

void EventBus::publish(Event event) {
    instance._queue.push(event);
}

Event EventBus::poll() {
    if(instance._queue.empty()){
        throw std::runtime_error{ "EventBus empty" };
    }

    auto event = instance._queue.front();
    instance._queue.pop();

    return  event;
}

bool EventBus::hasEvents() {
    return !instance._queue.empty();
}
