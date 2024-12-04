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
    static double now_;
    static double elapsed_;
    static double delta_;
};