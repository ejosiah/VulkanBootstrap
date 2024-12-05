#pragma once

#include <chrono>

class Time {
public:
    using Delta = std::chrono::milliseconds;
    using Elapsed = std::chrono::milliseconds;
    using Now = std::chrono::milliseconds;

    void tick();
    [[nodiscard]] Delta delta() const;
    [[nodiscard]] Elapsed elapsed() const;
    [[nodiscard]] Now now() const;

    static Time& instance() {
        static Time time{};
        return time;
    }

private:
    Time() = default;

    Now now_{};
    Elapsed elapsed_{};
    Delta delta_{};
};