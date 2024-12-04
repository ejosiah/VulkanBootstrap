#include <VulkanApplication.hpp>
#include "CubeScene.hpp"
#include <spdlog/spdlog.h>

int main() {
    auto app = VulkanApplication::bootStrap([](auto device, const auto& appState){
        return std::make_shared<CubeScene>(device, appState);
    });
    app.run();
    return 0;
}
