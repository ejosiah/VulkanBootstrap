#include "event/Events.hpp"

#include <stdexcept>

EventBus EventBus::instance{};

void EventBus::Publish(Event event) {
    instance.queue_.push(event);
}

Event EventBus::Poll() {
    if(instance.queue_.empty()){
        throw std::runtime_error{ "EventBus empty" };
    }

    auto event = instance.queue_.front();
    instance.queue_.pop();

    return  event;
}

bool EventBus::HasEvents() {
    return !instance.queue_.empty();
}

void Events::ClearScreen(float r, float g, float b, float a) {
    EventBus::Publish(ClearScreenEvent(r, g, b, a));
}
