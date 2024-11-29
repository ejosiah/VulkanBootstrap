#pragma once

#include <variant>
#include <queue>

struct InvalidateEvent{};
struct ClearScreenEvent{
    float r, g, b, a;
};

using Event = std::variant<InvalidateEvent>;

struct Events {
    static constexpr InvalidateEvent invalidate{};
};

class EventBus {
public:
    static void publish(Event event);

    static Event poll();

    static bool hasEvents();

private:
    EventBus() = default;

private:
    std::queue<Event> _queue;
    static EventBus instance;
};
