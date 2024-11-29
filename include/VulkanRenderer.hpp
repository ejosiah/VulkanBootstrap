#pragma once

#include "WindowInterface.hpp"
#include "VulkanInstance.hpp"
#include "VulkanDevice.hpp"
#include "VulkanDebugMessenger.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanCommandPool.hpp"

class VulkanRenderer {
public:
    VulkanRenderer(
            std::shared_ptr<Window> window,
            std::shared_ptr<VulkanInstance> instance,
            std::shared_ptr<VulkanDevice> device,
            std::unique_ptr<VulkanSwapchain> swapchain,
            VulkanSwapchainBuilder swapchainBuilder);


    void init();

    void renderFrame();

    void clearColor(float r, float g, float b, float a = 1);

    void stop();

    void invalidateSwapchain();

private:
    void createCommandPool();

    void createSynchronizationPrimitives();

    void initFrameBufferPrimitives();

    void recordScene(uint32_t imageIndex);

private:
    std::shared_ptr<Window> _window;
    std::shared_ptr<VulkanInstance> _instance;
    std::shared_ptr<VulkanDevice> _device;
    std::unique_ptr<VulkanSwapchain> _swapchain;
    VulkanSwapchainBuilder _swapchainBuilder;
    std::shared_ptr<VulkanCommandPool> _commandPool;
    std::vector<VkCommandBuffer> _renderCommandBuffer{};


    VkSemaphore _acquireImageSemaphore;
    VkSemaphore _renderingFinishedSemaphore;
    VkFence _inFlightFrame;
    VkSubmitInfo _renderSubmitInfo;
    VkPresentInfoKHR _presentInfo;

    VkRenderingInfo _renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };

    struct {
        VkImage image;
        std::vector<VkImageView> imageView;
        VkRenderingAttachmentInfo attachment;
    } _depthBuffer;

    struct {
        struct {
            VkImage image;
            VkImageView imageView;
        } _[10];
        std::vector<VkRenderingAttachmentInfo> attachment;
    } _colorBuffer;
    VkClearColorValue _clearColor{ };
    std::vector<VkClearColorValue> _clearColors{  {0, 1.f, 0, 1.f}, {1.f, 0, 0, 1.f}, {0, 0, 1.f, 1.f} };
};