#pragma once

#include <variant>
#include <queue>

#include <cinttypes>

struct InvalidateEvent{};
struct FrameBufferResizeEvent{
    uint32_t width, height;
};
struct ClearScreenEvent{
    float r, g, b, a;
};

using Event = std::variant<InvalidateEvent, FrameBufferResizeEvent, ClearScreenEvent>;

struct Events {
    static constexpr InvalidateEvent Invalidate{};

    static FrameBufferResizeEvent FrameBufferResized(int width, int height) {
        return FrameBufferResizeEvent{ static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

    static void ClearScreen(float r, float g, float b, float a = 1);
};

class EventBus {
public:
    static void Publish(Event event);

    static Event Poll();

    static bool HasEvents();

private:
    EventBus() = default;

private:
    std::queue<Event> queue_;
    static EventBus instance;
};
