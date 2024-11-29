#include "VulkanSwapchain.hpp"
#include <algorithm>
#include <spdlog/spdlog.h>
#include <stdexcept>
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

uint32_t VulkanSwapchain::imageCount() const {
    return _images.size();
}

VkImage VulkanSwapchain::getImage(uint32_t index) {
    assert(index < _images.size());
    return _images[index];
}

VkFormat VulkanSwapchain::format() const {
    return _format;
}

uint32_t VulkanSwapchain::width() const {
    return _width;
}

uint32_t VulkanSwapchain::height() const {
    return _height;
}

VulkanSwapchain::operator VkSwapchainKHR() const {
    return _swapchain;
}

VulkanSwapchain::operator const VkSwapchainKHR *() const {
    return &_swapchain;
}

VulkanSwapchainBuilder::VulkanSwapchainBuilder(const std::shared_ptr<VulkanDevice>& device, VkSurfaceKHR surface)
: _device(device)
, _surface(surface)
, _capabilities(device->getSurfaceCapabilities(surface))
, _supportedFormats(device->getSurfaceFormat(surface))
, _supportedModes(device->getSurfacePresentationsModes(surface)){
    _minImageCount = std::min(_capabilities.minImageCount + 1, _capabilities.maxImageCount);
    _format = _supportedFormats.front();
    _extent = { _capabilities.minImageExtent.width, _capabilities.minImageExtent.height};
}

VulkanSwapchainBuilder &VulkanSwapchainBuilder::setMinImageCount(uint32_t value) {
    _minImageCount = std::min(value + 1, _capabilities.maxImageCount);
    return *this;
}

VulkanSwapchainBuilder &VulkanSwapchainBuilder::setImageFormat(VkFormat format, VkColorSpaceKHR colorSpace) {
    bool supported = false;
    for(auto aFormat : _supportedFormats) {
        if(format == aFormat.format && colorSpace == aFormat.colorSpace) {
            supported = true;
            break;
        }
    }
    if(!supported) {
        spdlog::warn("requested format not supported, using default format");
        _format = _supportedFormats.front();
        return *this;
    }
    _format = { format, colorSpace };
    return *this;
}

VulkanSwapchainBuilder &VulkanSwapchainBuilder::setExtent(uint32_t width, uint32_t height) {
    _extent.width = std::clamp(width, _capabilities.minImageExtent.width, _capabilities.maxImageExtent.width);
    _extent.height = std::clamp(height, _capabilities.minImageExtent.height, _capabilities.maxImageExtent.height);
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
    VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };

    const auto format = getFormat();
    const auto presentMode = getPresentMode();

    createInfo.surface = _surface;
    createInfo.minImageCount = _minImageCount;
    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent = _extent;
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

    std::vector<VkImage> images(_minImageCount);
    vkGetSwapchainImagesKHR(_device->handle(), _swapchain, &_minImageCount, images.data());
    return std::make_unique<VulkanSwapchain>(_device->handle(), _swapchain, images, _extent, format.format);
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
