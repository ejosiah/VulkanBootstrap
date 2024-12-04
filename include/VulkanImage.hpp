#pragma once

#include <volk.h>
#include "vk_mem_alloc.h"

#include <memory>

class VulkanImage {
public:
    const VkImageCreateInfo spec;

    VulkanImage(VmaAllocator allocator, VmaAllocation allocation, VkImage image, const VkImageCreateInfo& spec);

    ~VulkanImage();

    operator VkImage() const;

private:
    VmaAllocator allocator_;
    VmaAllocation allocation_;
    VkImage image_;
};

class VulkanImageView {
public:
    const VkImageViewCreateInfo spec;

    VulkanImageView(VkDevice device, VkImageView imageView, const VkImageViewCreateInfo& spec);

    ~VulkanImageView();

    operator VkImageView() const;

private:
    VkDevice device_;
    VkImageView imageView_;
};

class VulkanImageCreator {
public:
    VulkanImageCreator(VkDevice device, VmaAllocator allocator);

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

    std::tuple<VkImage, VmaAllocation> create();

    std::unique_ptr<VulkanImage> make_unique();

    std::shared_ptr<VulkanImage> make_shared();

private:
    VkDevice device_{};
    VmaAllocator allocator_{};
    VkImageCreateInfo info_{ };
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

    std::unique_ptr<VulkanImageView> make_unique();

    std::shared_ptr<VulkanImageView> make_shared();

private:
    VkDevice device_{};
    VkImageViewCreateInfo info_;
};