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
    : window_(std::move(window))
    , instance_(std::move(instance))
    , debugMessenger_(std::move(debugMessenger))
    , device_(std::move(device))
    , renderer_(std::move(renderer))
    , scene_(std::move(scene))
    , appState_(appState){}

void VulkanApplication::run() {
    setup();

    while(window_->isActive()) {
        appState_.currentFrame_ = renderer_->currentFrame();
        processEvents();
        scene_->update();
        renderer_->renderFrame(scene_->record().front());
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
    renderer_->init();
    initState();
    scene_->init0();
}

void VulkanApplication::shutdown() {
    device_->wait();
    renderer_->stop();
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
                    renderer_->clearColor(e.r, e.g, e.b, e.a);
                }
        }, EventBus::Poll());
    }
}

void VulkanApplication::initState() {
    appState_.screenWidth_ = renderer_->width();
    appState_.screenHeight_ = renderer_->height();
    appState_.numFramesInFlight_ = renderer_->framesInFlight();
    appState_.screenSampleCount_ = renderer_->samples();
    appState_.screenFormat_ = renderer_->format();
    appState_.screenDepthFormat_ = renderer_->depthFormat();
    appState_.colorBufferCount_ = renderer_->colorBufferCount();
}

void VulkanApplication::invalidate() {
    renderer_->invalidateSwapchain();
    initState();
    scene_->refresh();
}
