#include "Types.hpp"
#include "VulkanImageCreator.hpp"

#include <stdexcept>

VulkanImageCreator::VulkanImageCreator(VkDevice device)
: _device(device)
, _info{
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .imageType = VK_IMAGE_TYPE_2D,
    .extent = {0, 0, 1},
    .mipLevels = 1,
    .arrayLayers = 1,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
}{}

VulkanImageCreator &VulkanImageCreator::flags(VkImageCreateFlagBits flags) {
    _info.flags = flags;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::imageType(VkImageType imageType) {
    _info.imageType = imageType;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::format(VkFormat format) {
    _info.format = format;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::extent(VkExtent3D extent) {
    _info.extent = extent;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::width(uint32 width) {
    _info.extent.width = width;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::height(uint32 height) {
    _info.extent.height = height;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::depth(uint32 depth) {
    _info.extent.depth = depth;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::mipLevels(int levels) {
    _info.mipLevels = levels;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::arrayLayers(int layers) {
    _info.arrayLayers =  layers;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::samples(VkSampleCountFlagBits samples) {
    _info.samples = samples;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::tiling(VkImageTiling tiling) {
    _info.tiling = tiling;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::usage(VkImageUsageFlagBits usage) {
    _info.usage = usage;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::sharingMode(VkSharingMode mode) {
    _info.sharingMode = mode;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::queueFamilyIndexCount(uint32 count) {
    _info.queueFamilyIndexCount = count;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::pQueueFamilyIndices(const uint32 *ptr) {
    _info.pQueueFamilyIndices = ptr;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::initialLayout(VkImageLayout layout) {
    _info.initialLayout = layout;
    return *this;
}

VkImage VulkanImageCreator::create() {
    VkImage image;
    auto result = vkCreateImage(_device, &_info, VK_NULL_HANDLE, &image);
    if(result != VK_SUCCESS) {
        throw std::runtime_error{ "unable to create image"};
    }
    return image;
}

VulkanImageViewCreator::VulkanImageViewCreator(VkDevice device)
: _device(device)
, _info{
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .viewType = VK_IMAGE_VIEW_TYPE_2D,
    .components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
    .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
}
{}

VulkanImageViewCreator &VulkanImageViewCreator::flags(VkImageViewCreateFlagBits flags) {
    _info.flags = flags;
    return *this;
}

VulkanImageViewCreator &VulkanImageViewCreator::image(VkImage image) {
    _info.image = image;
    return *this;
}

VulkanImageViewCreator &VulkanImageViewCreator::viewType(VkImageViewType viewType) {
    _info.viewType = viewType;
    return *this;
}

VulkanImageViewCreator &VulkanImageViewCreator::format(VkFormat format) {
    _info.format = format;
    return *this;
}

VulkanImageViewCreator &VulkanImageViewCreator::componentSwizzleR(VkComponentSwizzle swizzle) {
    _info.components.r = swizzle;
    return *this;
}


VulkanImageViewCreator &VulkanImageViewCreator::componentSwizzleG(VkComponentSwizzle swizzle) {
    _info.components.g = swizzle;
    return *this;
}


VulkanImageViewCreator &VulkanImageViewCreator::componentSwizzleB(VkComponentSwizzle swizzle) {
    _info.components.b = swizzle;
    return *this;
}


VulkanImageViewCreator &VulkanImageViewCreator::componentSwizzleA(VkComponentSwizzle swizzle) {
    _info.components.a = swizzle;
    return *this;
}

VulkanImageViewCreator &VulkanImageViewCreator::aspectMask(VkImageAspectFlagBits aspectMask) {
    _info.subresourceRange.aspectMask = aspectMask;
    return *this;
}

VulkanImageViewCreator &VulkanImageViewCreator::baseMipLevel(uint32 level) {
    _info.subresourceRange.baseMipLevel = level;
    return *this;
}

VulkanImageViewCreator &VulkanImageViewCreator::levelCount(uint32 count) {
    _info.subresourceRange.levelCount = count;
    return *this;
}

VulkanImageViewCreator &VulkanImageViewCreator::baseArrayLayer(uint32 layer) {
    _info.subresourceRange.baseArrayLayer = layer;
    return *this;
}

VulkanImageViewCreator &VulkanImageViewCreator::layerCount(uint32 count) {
    _info.subresourceRange.layerCount = count;
    return *this;
}

VkImageView VulkanImageViewCreator::create() {
    if(!_info.image){
        throw std::runtime_error{ "image is not yet defined" };
    }

        if(_info.format == VK_FORMAT_UNDEFINED) {
            throw std::runtime_error{ "format is undefined" };
        }

    VkImageView imageView;
    auto result = vkCreateImageView(_device, &_info, VK_NULL_HANDLE, &imageView);

    if(result != VK_SUCCESS){
        throw std::runtime_error{ "unable to create image view"};
    }

    return imageView;
}
