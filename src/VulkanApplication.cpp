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
    std::shared_ptr<Scene> scene)
    : _window(std::move(window))
    , _instance(std::move(instance))
    , _debugMessenger(std::move(debugMessenger))
    , _device(std::move(device))
    , _renderer(std::move(renderer))
    , _scene(std::move(scene)){}

void VulkanApplication::run() {
    setup();

    while(_window->isActive()) {
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

    auto instance =
        VulkanInstance::builder()
            .appName("Vulkan Bootstrap")
            .addExtensions(WindowInterface::extensions())
//            .addLayer("VK_LAYER_LUNARG_api_dump")
        .make_shared();

    volkLoadInstance(instance->handle());

    auto debugMessenger = VulkanDebugMessenger::createDebugMessenger(instance->handle());

    auto window = WindowInterface::make_shared(500, 500, "vulkan bootstrap");
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

    std::shared_ptr<Scene> scene = sceneFactory(device);
    auto renderer = std::make_shared<VulkanRenderer>(window, instance, device, std::move(swapchain), scBuilder);

    return { window, instance, debugMessenger, device, renderer, scene };
}

void VulkanApplication::setup() {
    Time::Init();
    _renderer->init();
    initProperties();
    _scene->set(_properties);
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
        auto event = EventBus::Poll();

        std::visit(overloaded{
                [&](const InvalidateEvent e) {
                    _renderer->invalidateSwapchain();
                },
                [&](const FrameBufferResizeEvent e) {
                    EventBus::Publish(Events::Invalidate);
                },
                [&](const ClearScreenEvent e) {
                    _renderer->clearColor(e.r, e.g, e.b, e.a);
                }
        }, event);
    }
}

void VulkanApplication::initProperties() {
    _properties = std::make_shared<Properties>();
    _properties->width = _renderer->width();
    _properties->height = _renderer->height();
    _properties->framesInFlight = _renderer->framesInFlight();
    _properties->samples = _renderer->samples();
    _properties->colorFormat = _renderer->format();
    _properties->depthFormat = _renderer->depthFormat();
}

void VulkanApplication::invalidate() {
    _renderer->invalidateSwapchain();
    initProperties();
    _scene->refresh();
}
