#include "Types.hpp"
#include "VulkanSwapchain.hpp"
#include <algorithm>
#include <spdlog/spdlog.h>
#include <utility>

VulkanSwapchain::VulkanSwapchain(VkDevice device, VkSwapchainKHR swapchain, std::vector<VkImage> images, VkExtent2D extent, VkFormat format)
: _device(device)
, _swapchain(swapchain)
, _images(std::move(images))
, _width(extent.width)
, _height(extent.height)
, _format(format){}

VulkanSwapchain::~VulkanSwapchain() {
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);
}

VulkanSwapchainBuilder VulkanSwapchain::builder(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface) {
    return { std::move(device), surface };
}

uint32 VulkanSwapchain::imageCount() const {
    return _images.size();
}

VkImage VulkanSwapchain::getImage(uint32 index) {
    assert(index < _images.size());
    return _images[index];
}

VkFormat VulkanSwapchain::format() const {
    return _format;
}

uint32 VulkanSwapchain::width() const {
    return _width;
}

uint32 VulkanSwapchain::height() const {
    return _height;
}

VulkanSwapchain::operator VkSwapchainKHR() const {
    return _swapchain;
}

VulkanSwapchain::operator const VkSwapchainKHR *() const {
    return &_swapchain;
}


VulkanSwapchainBuilder::VulkanSwapchainBuilder(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface)
: _device(std::move(device))
, _surface(surface)
{}

VulkanSwapchainBuilder &VulkanSwapchainBuilder::setMinImageCount(uint32 value) {
    _minImageCount = value + 1;
    return *this;
}

VulkanSwapchainBuilder &VulkanSwapchainBuilder::setImageFormat(VkFormat format, VkColorSpaceKHR colorSpace) {
    _format = { format, colorSpace };
    return *this;
}

VulkanSwapchainBuilder &VulkanSwapchainBuilder::setExtent(uint32 width, uint32 height) {
    _extent.width = width;
    _extent.height = height;
    return *this;
}

VulkanSwapchainBuilder &VulkanSwapchainBuilder::setPreTransform(VkSurfaceTransformFlagBitsKHR transform) {
    _preTransform = transform;
    return *this;
}

VulkanSwapchainBuilder &VulkanSwapchainBuilder::setCompositeAlpha(VkCompositeAlphaFlagBitsKHR compositeAlpha) {
    _compositeAlpha = compositeAlpha;
    return *this;
}

VulkanSwapchainBuilder &VulkanSwapchainBuilder::setPresentMode(VkPresentModeKHR mode) {
    _presentMode = mode;
    return *this;
}

std::unique_ptr<VulkanSwapchain> VulkanSwapchainBuilder::make_unique() {
    _capabilities = _device->getSurfaceCapabilities(_surface);
    _supportedFormats = _device->getSurfaceFormat(_surface);
    _supportedModes = _device->getSurfacePresentationsModes(_surface);

    VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };

    const auto format = getFormat();
    const auto presentMode = getPresentMode();
    const auto extent = getExtent();
    auto imageCount = std::min(_minImageCount, _capabilities.maxImageCount);

    createInfo.surface = _surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    createInfo.preTransform = _preTransform;
    createInfo.compositeAlpha = _compositeAlpha;
    createInfo.presentMode = presentMode;
    createInfo.clipped = true;
    createInfo.oldSwapchain = _oldSwapchain;

    auto result = vkCreateSwapchainKHR(_device->handle(), &createInfo, nullptr, &_swapchain);

    if(result != VK_SUCCESS){
        return {};
    }
    _oldSwapchain = _swapchain;

    std::vector<VkImage> images(imageCount);
    vkGetSwapchainImagesKHR(_device->handle(), _swapchain, &imageCount, images.data());
    return std::make_unique<VulkanSwapchain>(_device->handle(), _swapchain, images, extent, format.format);
}

VkSurfaceFormatKHR VulkanSwapchainBuilder::getFormat() const {
    bool supported = std::any_of(_supportedFormats.begin(), _supportedFormats.end(), [a=_format](auto b){
        return a.format == b.format && a.colorSpace == b.colorSpace;
    });

    if(supported){
        return _format;
    }

    return _supportedFormats.front();
}



VkPresentModeKHR VulkanSwapchainBuilder::getPresentMode() const {
    bool requestedModeAvailable = std::any_of(_supportedModes.begin(), _supportedModes.end(), [a=_presentMode](auto b){
        return a == b;
    });

    if(requestedModeAvailable){
        return _presentMode;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapchainBuilder::getExtent() const {
    return { std::clamp(_extent.width, _capabilities.minImageExtent.width, _capabilities.maxImageExtent.width),
        std::clamp(_extent.height, _capabilities.minImageExtent.height, _capabilities.maxImageExtent.height) };
}
