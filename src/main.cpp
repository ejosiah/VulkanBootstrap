#include <BootStrap.hpp>
#include "CubeScene.hpp"
#include <spdlog/spdlog.h>

int main() {
    auto app = BootStrap::application([](auto device, const auto& appState){
        return std::make_shared<CubeScene>(device, appState);
    });
    app.run();
    return 0;
}
