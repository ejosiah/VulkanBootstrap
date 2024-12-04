#include "Types.hpp"
#include "VulkanApplication.hpp"
#include "event/Events.hpp"
#include "Time.hpp"

#include <spdlog/spdlog.h>
#include <utility>

VulkanApplication::VulkanApplication(
    std::shared_ptr<Window> window,
    std::shared_ptr<VulkanInstance> instance,
    std::shared_ptr<VulkanDebugMessenger> debugMessenger,
    std::shared_ptr<VulkanDevice> device,
    std::shared_ptr<VulkanRenderer> renderer,
    std::shared_ptr<Scene> scene,
    AppState& appState)
    : _window(std::move(window))
    , _instance(std::move(instance))
    , _debugMessenger(std::move(debugMessenger))
    , _device(std::move(device))
    , _renderer(std::move(renderer))
    , _scene(std::move(scene))
    , _appState(appState){}

void VulkanApplication::run() {
    setup();

    while(_window->isActive()) {
        _appState.currentFrame_ = _renderer->currentFrame();
        processEvents();
        _scene->update();
        _renderer->renderFrame(_scene->record().front());
        Time::Tick();
    }

    shutdown();
}

VulkanApplication VulkanApplication::bootStrap(SceneFactory&& sceneFactory) {
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

    volkLoadInstance(instance->handle());

    auto debugMessenger = VulkanDebugMessenger::createDebugMessenger(instance->handle());

    auto device =
            VulkanDevice::builder(instance->handle())
                .addExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                .addExtension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
                .addQueue(VK_QUEUE_GRAPHICS_BIT)
                .addSurface(instance->surface())
            .make_shared();

    auto scBuilder = VulkanSwapchain::builder(device, instance->surface());
    auto swapchain =
        scBuilder
            .setImageFormat(VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            .setPresentMode(VK_PRESENT_MODE_IMMEDIATE_KHR)
        .make_unique();

    std::shared_ptr<Scene> scene = sceneFactory(device, AppState::instance());
    auto renderer = std::make_shared<VulkanRenderer>(window, instance, device, std::move(swapchain), scBuilder, VK_SAMPLE_COUNT_16_BIT);

    return { window, instance, debugMessenger, device, renderer, scene, AppState::instance() };
}

void VulkanApplication::setup() {
    Time::Init();
    _renderer->init();
    initState();
    _scene->init0();
}

void VulkanApplication::shutdown() {
    _device->wait();
    _renderer->stop();
    WindowInterface::disconnect();
}

void VulkanApplication::processEvents() {
    WindowInterface::pollEvents();

    while(EventBus::HasEvents()) {
        std::visit(overloaded{
                [&](const InvalidateEvent e) {
                    invalidate();
                },
                [&](const FrameBufferResizeEvent e) {
                    EventBus::Publish(Events::Invalidate);
                },
                [&](const ClearScreenEvent e) {
                    _renderer->clearColor(e.r, e.g, e.b, e.a);
                }
        }, EventBus::Poll());
    }
}

void VulkanApplication::initState() {
    _appState.screenWidth_ = _renderer->width();
    _appState.screenHeight_ = _renderer->height();
    _appState.numFramesInFlight_ = _renderer->framesInFlight();
    _appState.screenSampleCount_ = _renderer->samples();
    _appState.screenFormat_ = _renderer->format();
    _appState.screenDepthFormat_ = _renderer->depthFormat();
    _appState.colorBufferCount_ = _renderer->colorBufferCount();
}

void VulkanApplication::invalidate() {
    _renderer->invalidateSwapchain();
    initState();
    _scene->refresh();
}
