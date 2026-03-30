#include "BootStrap.hpp"

#include "data_structure/ResourcePool.hpp"
#include <stdexcept>

VulkanApplication BootStrap::application(SceneFactory &&sceneFactory) {
    WindowInterface::connect();
    auto result = volkInitialize();

    if(result != VK_SUCCESS){
        throw std::runtime_error("unable to load volk");
    }

    auto window = WindowInterface::make_shared(500, 500, "vulkan bootstrap");

    auto instance =
            VulkanInstance::builder()
                    .appName("Vulkan Bootstrap")
                    .addWindow(window)
//            .addLayer("VK_LAYER_LUNARG_api_dump")
                    .make_shared();

    if(!instance) {
        throw std::runtime_error("failed to initialize Vulkan instance");
    }

    volkLoadInstance(instance->handle());

    auto debugMessenger = VulkanDebugMessenger::createDebugMessenger(instance->handle());

    auto device =
        VulkanDevice::builder(instance->handle())
            .addExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
            .addExtension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
            .addQueue(VK_QUEUE_GRAPHICS_BIT)
            .addSurface(instance->surface())
        .make_shared();

    if(!device) {
        throw std::runtime_error("failed to initialize Vulkan device");
    }

    auto scBuilder = VulkanSwapchain::builder(device, instance->surface());
    auto swapchain =
        scBuilder
            .setImageFormat(VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            .setPresentMode(VK_PRESENT_MODE_IMMEDIATE_KHR)
        .make_unique();

    if(!swapchain) {
        throw std::runtime_error("failed to create swapchain");
    }

    auto& batchSubmission = BatchSubmission::instance(device->getGraphicsQueue());
    std::shared_ptr<Scene> scene = sceneFactory(device, AppState::instance(), batchSubmission);
    auto renderer = std::make_shared<VulkanRenderer>(window, instance, device, std::move(swapchain), scBuilder, batchSubmission, VK_SAMPLE_COUNT_16_BIT);

    return { window, instance, debugMessenger, device, renderer, scene, AppState::instance() };
}
