#pragma once

class Time {
public:
    void tick();
    [[nodiscard]] double delta() const;
    [[nodiscard]] double elapsed() const;
    [[nodiscard]] double now() const;

    static Time& instance() {
        static Time time{};
        return time;
    }

private:
    Time() = default;

    double now_{};
    double elapsed_{};
    double delta_{};
};