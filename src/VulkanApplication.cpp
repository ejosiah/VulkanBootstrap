#include "VulkanApplication.hpp"
#include "event/Events.hpp"

#include <spdlog/spdlog.h>
#include <utility>

VulkanApplication::VulkanApplication(
    std::shared_ptr<Window> window,
    std::shared_ptr<VulkanInstance> instance,
    std::shared_ptr<VulkanDebugMessenger> debugMessenger,
    std::shared_ptr<VulkanDevice> device,
    std::shared_ptr<VulkanRenderer> renderer)
    : _window(std::move(window))
    , _instance(std::move(instance))
    , _debugMessenger(std::move(debugMessenger))
    , _device(std::move(device))
    , _renderer(std::move(renderer)){}

void VulkanApplication::run() {
    setup();

    while(_window->isActive()) {
        processEvents();
        _renderer->renderFrame();
        tick();
    }

    shutdown();
}

VulkanApplication VulkanApplication::bootStrap() {
    WindowInterface::connect();
    auto result = volkInitialize();

    if(result != VK_SUCCESS){
        throw std::runtime_error("unable to load volk");
    }

    auto instance =
        VulkanInstance::builder()
            .appName("Vulkan Bootstrap")
            .addExtensions(WindowInterface::extensions())
//            .addLayer("VK_LAYER_LUNARG_api_dump")
        .make_shared();

    volkLoadInstance(instance->handle());

    auto debugMessenger = VulkanDebugMessenger::createDebugMessenger(instance->handle());

    auto window = WindowInterface::make_shared(1024, 720, "vulkan bootstrap");
    auto surface = window->createSurface(instance->handle());

    auto device =
            VulkanDevice::builder(instance->handle())
                .addExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                .addExtension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
                .addQueue(VK_QUEUE_GRAPHICS_BIT)
                .addSurface(surface)
            .make_shared();

    auto scBuilder = VulkanSwapchain::builder(device, surface);
    auto swapchain =
            scBuilder
                    .setImageFormat(VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    .make_unique();
    auto renderer = std::make_shared<VulkanRenderer>(window, instance, device, std::move(swapchain), scBuilder);

    return { window, instance, debugMessenger, device, renderer };
}

void VulkanApplication::setup() {
    _renderer->init();
}

void VulkanApplication::shutdown() {
    _device->wait();
    _renderer->stop();
    WindowInterface::disconnect();
}

void VulkanApplication::tick() {
    _elapsedTime = glfwGetTime();
}

// helper type for the visitor #4
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

void VulkanApplication::processEvents() {
    WindowInterface::pollEvents();

    while(EventBus::hasEvents()) {
        auto event = EventBus::poll();

        std::visit(overloaded{
                [&](const InvalidateEvent event) {
                    _renderer->invalidateSwapchain();
                }
        }, event);
    }
}
