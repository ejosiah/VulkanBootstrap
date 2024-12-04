#include <GLFW/glfw3.h>
#include "Time.hpp"

double Time::now_;
double Time::elapsed_;
double Time::delta_;

void Time::Init() {
    now_ = 0;
    elapsed_ = 0;
    delta_ = 0;
}

void Time::Tick() {
    now_ = glfwGetTime();
    delta_ = now_ - elapsed_;
    elapsed_ += delta_;
}

double Time::Elapsed() {
    return elapsed_;
}

double Time::Delta() {
    return delta_;
}

double Time::Now() {
    return now_;
}
