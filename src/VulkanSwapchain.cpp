#include "Types.hpp"
#include "VulkanSwapchain.hpp"
#include <algorithm>
#include <limits>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <utility>

VulkanSwapchain::VulkanSwapchain(VkDevice device, VkSwapchainKHR swapchain, std::vector<VkImage> images, VkExtent2D extent, VkFormat format)
: device_(device)
, swapchain_(swapchain)
, images_(std::move(images))
, width_(extent.width)
, height_(extent.height)
, format_(format){}

VulkanSwapchain::~VulkanSwapchain() {
    vkDestroySwapchainKHR(device_, swapchain_, nullptr);
}

VulkanSwapchainBuilder VulkanSwapchain::builder(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface) {
    return { std::move(device), surface };
}

uint32 VulkanSwapchain::imageCount() const {
    return images_.size();
}

VkImage VulkanSwapchain::getImage(uint32 index) {
    assert(index < images_.size());
    return images_[index];
}

VkFormat VulkanSwapchain::format() const {
    return format_;
}

uint32 VulkanSwapchain::width() const {
    return width_;
}

uint32 VulkanSwapchain::height() const {
    return height_;
}

VulkanSwapchain::operator VkSwapchainKHR() const {
    return swapchain_;
}

VulkanSwapchain::operator const VkSwapchainKHR *() const {
    return &swapchain_;
}


VulkanSwapchainBuilder::VulkanSwapchainBuilder(std::shared_ptr<VulkanDevice> device, VkSurfaceKHR surface)
: device_(std::move(device))
, surface_(surface)
{}

VulkanSwapchainBuilder &VulkanSwapchainBuilder::setMinImageCount(uint32 value) {
    minImageCount_ = value + 1;
    return *this;
}

VulkanSwapchainBuilder &VulkanSwapchainBuilder::setImageFormat(VkFormat format, VkColorSpaceKHR colorSpace) {
    format_ = { format, colorSpace };
    return *this;
}

VulkanSwapchainBuilder &VulkanSwapchainBuilder::setExtent(uint32 width, uint32 height) {
    extent_.width = width;
    extent_.height = height;
    return *this;
}

VulkanSwapchainBuilder &VulkanSwapchainBuilder::setPreTransform(VkSurfaceTransformFlagBitsKHR transform) {
    preTransform_ = transform;
    return *this;
}

VulkanSwapchainBuilder &VulkanSwapchainBuilder::setCompositeAlpha(VkCompositeAlphaFlagBitsKHR compositeAlpha) {
    compositeAlpha_ = compositeAlpha;
    return *this;
}

VulkanSwapchainBuilder &VulkanSwapchainBuilder::setPresentMode(VkPresentModeKHR mode) {
    presentMode_ = mode;
    return *this;
}

std::unique_ptr<VulkanSwapchain> VulkanSwapchainBuilder::make_unique() {
    if(surface_ == VK_NULL_HANDLE) {
        throw std::runtime_error{"cannot create swapchain without a presentation surface"};
    }

    capabilities_ = device_->getSurfaceCapabilities(surface_);
    supportedFormats_ = device_->getSurfaceFormat(surface_);
    supportedModes_ = device_->getSurfacePresentationsModes(surface_);

    if(supportedFormats_.empty()) {
        throw std::runtime_error{"no swapchain surface formats available"};
    }

    if(supportedModes_.empty()) {
        throw std::runtime_error{"no swapchain present modes available"};
    }

    VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };

    const auto format = getFormat();
    const auto presentMode = getPresentMode();
    const auto extent = getExtent();
    auto imageCount = getMinImageCount();

    createInfo.surface = surface_;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    createInfo.preTransform = getPreTransform();
    createInfo.compositeAlpha = getCompositeAlpha();
    createInfo.presentMode = presentMode;
    createInfo.clipped = true;
    createInfo.oldSwapchain = oldSwapchain_;

    auto result = vkCreateSwapchainKHR(device_->handle(), &createInfo, nullptr, &swapchain_);

    if(result != VK_SUCCESS){
        throw std::runtime_error("unable to create Vulkan swapchain");
    }
    oldSwapchain_ = swapchain_;

    std::vector<VkImage> images(imageCount);
    vkGetSwapchainImagesKHR(device_->handle(), swapchain_, &imageCount, images.data());
    return std::make_unique<VulkanSwapchain>(device_->handle(), swapchain_, images, extent, format.format);
}

VkSurfaceFormatKHR VulkanSwapchainBuilder::getFormat() const {
    bool supported = std::any_of(supportedFormats_.begin(), supportedFormats_.end(), [a=format_](auto b){
        return a.format == b.format && a.colorSpace == b.colorSpace;
    });

    if(supported){
        return format_;
    }

    return supportedFormats_.front();
}



VkPresentModeKHR VulkanSwapchainBuilder::getPresentMode() const {
    bool requestedModeAvailable = std::any_of(supportedModes_.begin(), supportedModes_.end(), [a=presentMode_](auto b){
        return a == b;
    });

    if(requestedModeAvailable){
        return presentMode_;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

uint32 VulkanSwapchainBuilder::getMinImageCount() const {
    auto imageCount = std::max(minImageCount_, capabilities_.minImageCount);
    if(capabilities_.maxImageCount == 0) {
        return imageCount;
    }
    return std::min(imageCount, capabilities_.maxImageCount);
}

VkExtent2D VulkanSwapchainBuilder::getExtent() const {
    if(capabilities_.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities_.currentExtent;
    }

    return {
        std::clamp(extent_.width, capabilities_.minImageExtent.width, capabilities_.maxImageExtent.width),
        std::clamp(extent_.height, capabilities_.minImageExtent.height, capabilities_.maxImageExtent.height)
    };
}

VkSurfaceTransformFlagBitsKHR VulkanSwapchainBuilder::getPreTransform() const {
    if((capabilities_.supportedTransforms & preTransform_) != 0) {
        return preTransform_;
    }
    return capabilities_.currentTransform;
}

VkCompositeAlphaFlagBitsKHR VulkanSwapchainBuilder::getCompositeAlpha() const {
    if((capabilities_.supportedCompositeAlpha & compositeAlpha_) != 0) {
        return compositeAlpha_;
    }

    constexpr VkCompositeAlphaFlagBitsKHR fallbacks[] = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR
    };

    for(auto candidate : fallbacks) {
        if((capabilities_.supportedCompositeAlpha & candidate) != 0) {
            return candidate;
        }
    }

    return compositeAlpha_;
}
