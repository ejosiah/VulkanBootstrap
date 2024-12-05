#include "Types.hpp"
#include "Time.hpp"
#include <GLFW/glfw3.h>


void Time::tick() {
    now_ = std::chrono::milliseconds{ to<int64>(1000.f * glfwGetTime()) };
    delta_ = now_ - elapsed_;
    elapsed_ += delta_;
}

Time::Elapsed Time::elapsed() const {
    return elapsed_;
}

Time::Delta Time::delta() const {
    return delta_;
}

Time::Now Time::now() const {
    return now_;
}
