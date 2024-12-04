#pragma once

class Time {
public:
    static void Init();

    static void Tick();

    static double Delta();

    static double Elapsed();

    static double Now();

private:
    Time() = default;

private:
    static double _now;
    static double _elapsed;
    static double _delta;
};