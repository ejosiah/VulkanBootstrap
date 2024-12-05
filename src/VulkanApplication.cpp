#include "Types.hpp"
#include "VulkanApplication.hpp"
#include "event/Events.hpp"
#include "Time.hpp"

#include <spdlog/spdlog.h>
#include <utility>
#include <WindowInterface.hpp>

class VulkanApplication::Impl {
public:
    Impl(
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

    void run(){
        setup();
        while(window_->isActive()) {
            appState_.currentFrame_ = renderer_->currentFrame();
            processEvents();
            scene_->update();
            renderer_->renderFrame(scene_->record().front());
            Time::instance().tick();
        }

        shutdown();
    }

    void shutdown(){
        device_->wait();
        renderer_->stop();
        WindowInterface::disconnect();
    }

    void processEvents(){
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

    void setup() {
        renderer_->init();
        initState();
        scene_->init0();
    }

    void initState() {
        appState_.screenWidth_ = renderer_->width();
        appState_.screenHeight_ = renderer_->height();
        appState_.numFramesInFlight_ = renderer_->framesInFlight();
        appState_.screenSampleCount_ = renderer_->samples();
        appState_.screenFormat_ = renderer_->format();
        appState_.screenDepthFormat_ = renderer_->depthFormat();
        appState_.colorBufferCount_ = renderer_->colorBufferCount();
    }

    void invalidate(){
        renderer_->invalidateSwapchain();
        initState();
        scene_->refresh();
    }

private:
    std::shared_ptr<VulkanInstance> instance_;
    std::shared_ptr<Window> window_;
    std::shared_ptr<VulkanDebugMessenger> debugMessenger_;
    std::shared_ptr<VulkanDevice> device_;
    std::shared_ptr<VulkanRenderer> renderer_;
    std::shared_ptr<Scene> scene_;
    AppState& appState_;
};

VulkanApplication::VulkanApplication(
    std::shared_ptr<Window> window,
    std::shared_ptr<VulkanInstance> instance,
    std::shared_ptr<VulkanDebugMessenger> debugMessenger,
    std::shared_ptr<VulkanDevice> device,
    std::shared_ptr<VulkanRenderer> renderer,
    std::shared_ptr<Scene> scene,
    AppState& appState)
    : pimpl(new Impl(std::move(window)
    , (std::move(instance))
    , (std::move(debugMessenger))
    , (std::move(device))
    , (std::move(renderer))
    , (std::move(scene))
    , (appState))){}

void VulkanApplication::run() {
    pimpl->run();
}

void VulkanApplication::shutdown() {
    pimpl->shutdown();
}

void VulkanApplication::processEvents() {
    pimpl->processEvents();
}

VulkanApplication::~VulkanApplication() {
    delete pimpl;
}

