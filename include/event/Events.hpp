#pragma once

#include <variant>
#include <queue>

struct InvalidateEvent{};
struct FrameBufferResizeEvent{
    uint32_t width, height;
};
struct ClearScreenEvent{
    float r, g, b, a;
};

using Event = std::variant<InvalidateEvent, FrameBufferResizeEvent>;

struct Events {
    static constexpr InvalidateEvent invalidate{};

    static FrameBufferResizeEvent frameBufferResized(int width, int height) {
        return FrameBufferResizeEvent{ static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }
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
