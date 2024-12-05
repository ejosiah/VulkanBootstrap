#pragma once

#include <VulkanInstance.hpp>
#include <WindowInterface.hpp>
#include <VulkanDevice.hpp>
#include <VulkanDebugMessenger.hpp>
#include <VulkanSwapchain.hpp>
#include "VulkanCommandPool.hpp"
#include "VulkanRenderer.hpp"
#include "Scene.hpp"
#include "AppState.hpp"

#include <functional>
#include <memory>

class VulkanApplication {
    class Impl;
    Impl* pimpl;
public:
    using SceneFactory = std::function<std::shared_ptr<Scene>(std::shared_ptr<VulkanDevice>, const AppState&)>;

    VulkanApplication(
        std::shared_ptr<Window> window,
        std::shared_ptr<VulkanInstance> instance,
        std::shared_ptr<VulkanDebugMessenger> debugMessenger,
        std::shared_ptr<VulkanDevice> device,
        std::shared_ptr<VulkanRenderer> renderer,
        std::shared_ptr<Scene> scene,
        AppState& appState);

    ~VulkanApplication();

    void run();

   void shutdown();

   void processEvents();

   static VulkanApplication bootStrap(SceneFactory&& sceneFactory);

};