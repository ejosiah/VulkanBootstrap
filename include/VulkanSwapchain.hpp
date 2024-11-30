#pragma once

#include <volk.h>
#include <VulkanDevice.hpp>

class VulkanSwapchainBuilder;

class VulkanSwapchain {
public:
    VulkanSwapchain(VkDevice device, VkSwapchainKHR swapchain, std::vector<VkImage> images, VkExtent2D extent, VkFormat format);

    ~VulkanSwapchain();

    static VulkanSwapchainBuilder builder(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface);

    uint32_t imageCount() const;

    VkImage getImage(uint32_t index);

    VkFormat format() const;

    [[nodiscard]] uint32_t width() const;

    uint32_t  height() const;

    operator VkSwapchainKHR() const;

    operator const VkSwapchainKHR*() const;

private:
    VkDevice _device;
    VkSwapchainKHR _swapchain;
    std::vector<VkImage> _images;
    VkFormat _format{};
    uint32_t _width{};
    uint32_t _height{};
};

class VulkanSwapchainBuilder {
public:
    VulkanSwapchainBuilder(const std::shared_ptr<VulkanDevice>& device, VkSurfaceKHR surfaceKhr);

    VulkanSwapchainBuilder& setMinImageCount(uint32_t value);

    VulkanSwapchainBuilder& setImageFormat(VkFormat format, VkColorSpaceKHR colorSpace);

    VulkanSwapchainBuilder& setExtent(uint32_t width, uint32_t height);

    VulkanSwapchainBuilder& setPreTransform(VkSurfaceTransformFlagBitsKHR transform);

    VulkanSwapchainBuilder& setCompositeAlpha(VkCompositeAlphaFlagBitsKHR compositeAlpha);

    VulkanSwapchainBuilder& setPresentMode(VkPresentModeKHR mode);

    std::unique_ptr<VulkanSwapchain> make_unique();

private:
    VkSurfaceFormatKHR getFormat() const;

    VkPresentModeKHR  getPresentMode() const;

    VkExtent2D  getExtent() const;

private:
    std::shared_ptr<VulkanDevice> _device;
    VkSurfaceKHR _surface{};
    VkSurfaceCapabilitiesKHR _capabilities{};
    std::vector<VkSurfaceFormatKHR>  _supportedFormats;
    std::vector<VkPresentModeKHR > _supportedModes;
    VkSwapchainKHR _swapchain{};
    VkSwapchainKHR _oldSwapchain{};
    uint32_t _minImageCount{2};
    VkSurfaceFormatKHR _format{};
    VkExtent2D _extent{};
    VkPresentModeKHR _presentMode{VK_PRESENT_MODE_FIFO_KHR};
    VkSurfaceTransformFlagBitsKHR _preTransform{VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR};
    VkCompositeAlphaFlagBitsKHR _compositeAlpha{VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR};

};