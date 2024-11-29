#include <VulkanApplication.hpp>

#include <spdlog/spdlog.h>
#include <format>

#include <chrono>
#include <thread>

int main() {
    auto app = VulkanApplication::bootStrap();
    app.run();
    return 0;
}
