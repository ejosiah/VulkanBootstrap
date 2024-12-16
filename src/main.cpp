#include <BootStrap.hpp>
#include "CubeScene.hpp"
#include <spdlog/spdlog.h>
#include <spirv_reflect.h>
#include "io/IO.hpp"

int main() {
    BootStrap::application([](auto device, const auto& appState, auto batchSubmission){
        return std::make_shared<CubeScene>(device, appState, batchSubmission);
    }).run();
    return 0;
}
