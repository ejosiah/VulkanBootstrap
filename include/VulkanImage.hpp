#pragma once

#include <volk.h>

class VulkanImageCreator {
public:
    VulkanImageCreator(VkDevice device);

    VulkanImageCreator& flags(VkImageCreateFlagBits flags);

    VulkanImageCreator& imageType(VkImageType imageType);

    VulkanImageCreator& format(VkFormat format);

    VulkanImageCreator& extent(VkExtent3D extent);

    VulkanImageCreator& width(uint32_t width);

    VulkanImageCreator& height(uint32_t height);

    VulkanImageCreator& depth(uint32_t depth);

    VulkanImageCreator& mipLevels(int levels);

    VulkanImageCreator& arrayLayers(int layers);

    VulkanImageCreator& samples(VkSampleCountFlagBits samples);

    VulkanImageCreator& tiling(VkImageTiling tiling);

    VulkanImageCreator& usage(VkImageUsageFlagBits usage);

    VulkanImageCreator& sharingMode(VkSharingMode mode);

    VulkanImageCreator& queueFamilyIndexCount(uint32_t count);

    VulkanImageCreator& pQueueFamilyIndices(const uint32_t* ptr);

    VulkanImageCreator& initialLayout(VkImageLayout layout);

    VkImage create();

private:
    VkDevice _device{};
    VkImageCreateInfo _info{ };
};

class VulkanImageViewCreator{
public:
    VulkanImageViewCreator(VkDevice device);

    VulkanImageViewCreator& flags(VkImageViewCreateFlagBits flags);

    VulkanImageViewCreator& image(VkImage image);

    VulkanImageViewCreator& viewType(VkImageViewType viewType);

    VulkanImageViewCreator& format(VkFormat format);

    VulkanImageViewCreator& componentSwizzleR(VkComponentSwizzle swizzle);

    VulkanImageViewCreator& componentSwizzleG(VkComponentSwizzle swizzle);

    VulkanImageViewCreator& componentSwizzleB(VkComponentSwizzle swizzle);

    VulkanImageViewCreator& componentSwizzleA(VkComponentSwizzle swizzle);

    VulkanImageViewCreator& aspectMask(VkImageAspectFlagBits aspectMask);

    VulkanImageViewCreator& baseMipLevel(uint32_t level);

    VulkanImageViewCreator& levelCount(uint32_t count);

    VulkanImageViewCreator& baseArrayLayer(uint32_t layer);

    VulkanImageViewCreator& layerCount(uint32_t count);

    VkImageView create();

private:
    VkDevice _device{};
    VkImageViewCreateInfo _info;
};