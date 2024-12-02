#include <GLFW/glfw3.h>
#include "Time.hpp"

double Time::_now;
double Time::_elapsed;
double Time::_delta;

void Time::Init() {
    _now = 0;
    _elapsed = 0;
    _delta = 0;
}

void Time::Tick() {
    _now = glfwGetTime();
    _delta = _now - _elapsed;
    _elapsed += _delta;
}

double Time::Elapsed() {
    return _elapsed;
}

double Time::Delta() {
    return _delta;
}

double Time::Now() {
    return _now;
}
