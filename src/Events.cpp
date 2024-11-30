#include "event/Events.hpp"

#include <stdexcept>

EventBus EventBus::instance{};

void EventBus::Publish(Event event) {
    instance._queue.push(event);
}

Event EventBus::Poll() {
    if(instance._queue.empty()){
        throw std::runtime_error{ "EventBus empty" };
    }

    auto event = instance._queue.front();
    instance._queue.pop();

    return  event;
}

bool EventBus::HasEvents() {
    return !instance._queue.empty();
}

void Events::ClearScreen(float r, float g, float b, float a) {
    EventBus::Publish(ClearScreenEvent(r, g, b, a));
}
