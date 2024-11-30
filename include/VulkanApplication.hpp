#pragma once

#include <VulkanInstance.hpp>
#include <WindowInterface.hpp>
#include <VulkanDevice.hpp>
#include <VulkanDebugMessenger.hpp>
#include <VulkanSwapchain.hpp>
#include "VulkanCommandPool.hpp"
#include "VulkanRenderer.hpp"
#include "Scene.hpp"

class VulkanApplication {
public:
    VulkanApplication(
        std::shared_ptr<Window> window,
        std::shared_ptr<VulkanInstance> instance,
        std::shared_ptr<VulkanDebugMessenger> debugMessenger,
        std::shared_ptr<VulkanDevice> device,
        std::shared_ptr<VulkanRenderer> renderer,
        std::shared_ptr<Scene> scene);

    void run();

   void shutdown();

   void processEvents();

   static VulkanApplication bootStrap();

private:
    void setup();

private:
    std::shared_ptr<Window> _window;
    std::shared_ptr<VulkanInstance> _instance;
    std::shared_ptr<VulkanDebugMessenger> _debugMessenger;
    std::shared_ptr<VulkanDevice> _device;
    std::shared_ptr<Scene> _scene;
    std::shared_ptr<VulkanRenderer> _renderer;
};