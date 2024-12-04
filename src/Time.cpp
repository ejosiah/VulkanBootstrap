#include <GLFW/glfw3.h>
#include "Time.hpp"


void Time::tick() {
    now_ = glfwGetTime();
    delta_ = now_ - elapsed_;
    elapsed_ += delta_;
}

double Time::elapsed() const {
    return elapsed_;
}

double Time::delta() const {
    return delta_;
}

double Time::now() const {
    return now_;
}
