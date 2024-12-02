#include <VulkanApplication.hpp>
#include "CubeScene.hpp"
#include <spdlog/spdlog.h>

int main() {
    auto app = VulkanApplication::bootStrap([](auto device){ return std::make_shared<CubeScene>(device); });
    app.run();
    return 0;
}
