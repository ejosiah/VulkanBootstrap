#pragma once

#include "WindowInterface.hpp"
#include "VulkanInstance.hpp"
#include "VulkanDevice.hpp"
#include "VulkanDebugMessenger.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanCommandPool.hpp"
#include "VulkanImage.hpp"
#include <array>

class VulkanRenderer {
public:
    VulkanRenderer(
            std::shared_ptr<Window> window,
            std::shared_ptr<VulkanInstance> instance,
            std::shared_ptr<VulkanDevice> device,
            std::unique_ptr<VulkanSwapchain> swapchain,
            VulkanSwapchainBuilder swapchainBuilder,
            VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);


    void init();

    void renderFrame(VkCommandBuffer commandBuffer);

    void clearColor(float r, float g, float b, float a = 1);

    void stop();

    void invalidateSwapchain();

    [[nodiscard]] uint32 width() const;

    [[nodiscard]] uint32 height() const;

    [[nodiscard]] uint32 framesInFlight() const;

    [[nodiscard]] VkSampleCountFlagBits samples() const;

    [[nodiscard]] VkFormat format() const;

    [[nodiscard]] VkFormat depthFormat() const;

    [[nodiscard]] uint32 colorBufferCount() const;


private:
    void createCommandPool();

    void createSynchronizationPrimitives();

    void destroySynchronizationPrimitives();

    void initFrameBufferPrimitives();

    void recordScene(VkCommandBuffer sceneCommandBuffer);

    static VkSampleCountFlagBits ensureSampleCount(VkSampleCountFlagBits samples, const std::shared_ptr<VulkanDevice>& device);

private:
    static constexpr int MAX_IN_FLIGHT_FRAMES = 2;
    std::shared_ptr<Window> _window;
    std::shared_ptr<VulkanInstance> _instance;
    std::shared_ptr<VulkanDevice> _device;
    std::unique_ptr<VulkanSwapchain> _swapchain;
    VulkanSwapchainBuilder _swapchainBuilder;
    std::shared_ptr<VulkanCommandPool> _commandPool;
    std::vector<VkCommandBuffer> _renderCommandBuffer{};
    VkSampleCountFlagBits _samples;


    std::array<VkSemaphore, MAX_IN_FLIGHT_FRAMES> _acquireImageSemaphore;
    std::array<VkSemaphore, MAX_IN_FLIGHT_FRAMES> _renderingFinishedSemaphore;
    std::array<VkFence, MAX_IN_FLIGHT_FRAMES> _inFlightFrame;
    int _currentFrame{};
    VkSubmitInfo _renderSubmitInfo;
    VkPresentInfoKHR _presentInfo;

    VkRenderingInfo _renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };


    struct {
        std::unique_ptr<VulkanImage> image;
        std::vector<std::unique_ptr<VulkanImageView>> imageView;
        VkRenderingAttachmentInfo attachment;
    } _depthBuffer;

    struct {
        struct {
            VkImage image;
            std::unique_ptr<VulkanImageView> imageView;
            std::unique_ptr<VulkanImageView> resolveImageView;
        } _[10];
        std::vector<VkRenderingAttachmentInfo> attachment;
        std::vector<std::shared_ptr<VulkanImage>> msaaImages;
    } _colorBuffer;
    VkClearColorValue _clearColor{0, 0, 0, 1};

};