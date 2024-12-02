#include <VulkanApplication.hpp>
#include "CubeScene.hpp"
#include <spdlog/spdlog.h>

int main() {
#ifdef GLM_FORCE_DEPTH_ZERO_TO_ONE
    spdlog::info("GLM_FORCE_DEPTH_ZERO_TO_ONE defined");
#else
    spdlog::info("GLM_FORCE_DEPTH_ZERO_TO_ONE not defined");
#endif
    auto app = VulkanApplication::bootStrap([](auto device){ return std::make_shared<CubeScene>(device); });
    app.run();
    return 0;
}
