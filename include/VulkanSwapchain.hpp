#pragma once

#include <volk.h>
#include <VulkanDevice.hpp>

class VulkanSwapchainBuilder;

class VulkanSwapchain {
public:
    VulkanSwapchain(VkDevice device, VkSwapchainKHR swapchain, std::vector<VkImage> images, VkExtent2D extent, VkFormat format);

    ~VulkanSwapchain();

    static VulkanSwapchainBuilder builder(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface);

    [[nodiscard]] uint32_t imageCount() const;

    VkImage getImage(uint32_t index);

    [[nodiscard]] VkFormat format() const;

    [[nodiscard]] uint32_t width() const;

    [[nodiscard]] uint32_t  height() const;

    operator VkSwapchainKHR() const;

    operator const VkSwapchainKHR*() const;

private:
    VkDevice device_;
    VkSwapchainKHR swapchain_;
    std::vector<VkImage> images_;
    VkFormat format_{};
    uint32_t width_{};
    uint32_t height_{};
};

class VulkanSwapchainBuilder {
public:
    VulkanSwapchainBuilder(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surfaceKhr);

    [[maybe_unused]] VulkanSwapchainBuilder& setMinImageCount(uint32_t value);

    [[maybe_unused]] VulkanSwapchainBuilder& setImageFormat(VkFormat format, VkColorSpaceKHR colorSpace);

    [[maybe_unused]] VulkanSwapchainBuilder& setExtent(uint32_t width, uint32_t height);

    [[maybe_unused]] VulkanSwapchainBuilder& setPreTransform(VkSurfaceTransformFlagBitsKHR transform);

    [[maybe_unused]] VulkanSwapchainBuilder& setCompositeAlpha(VkCompositeAlphaFlagBitsKHR compositeAlpha);

    [[maybe_unused]] VulkanSwapchainBuilder& setPresentMode(VkPresentModeKHR mode);

    std::unique_ptr<VulkanSwapchain> make_unique();

private:
    [[nodiscard]] VkSurfaceFormatKHR getFormat() const;

    [[nodiscard]] VkPresentModeKHR  getPresentMode() const;

    [[nodiscard]] VkExtent2D  getExtent() const;

private:
    std::shared_ptr<VulkanDevice> device_;
    VkSurfaceKHR surface_{};
    VkSurfaceCapabilitiesKHR capabilities_{};
    std::vector<VkSurfaceFormatKHR>  supportedFormats_;
    std::vector<VkPresentModeKHR > supportedModes_;
    VkSwapchainKHR swapchain_{};
    VkSwapchainKHR oldSwapchain_{};
    uint32_t minImageCount_{2};
    VkSurfaceFormatKHR format_{};
    VkExtent2D extent_{};
    VkPresentModeKHR presentMode_{VK_PRESENT_MODE_FIFO_KHR};
    VkSurfaceTransformFlagBitsKHR preTransform_{VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR};
    VkCompositeAlphaFlagBitsKHR compositeAlpha_{VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR};

};