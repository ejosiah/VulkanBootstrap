#pragma once

#include <volk.h>
#include "vk_mem_alloc.h"

#include <memory>

class VulkanImage {
public:
    static constexpr VkObjectType  ObjectType = VK_OBJECT_TYPE_IMAGE;
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
    static constexpr VkObjectType  ObjectType = VK_OBJECT_TYPE_IMAGE_VIEW;
    const VkImageViewCreateInfo spec;

    VulkanImageView(VkDevice device, VkImageView imageView, const VkImageViewCreateInfo& spec);

    ~VulkanImageView();

    operator VkImageView() const;

private:
    VkDevice device_;
    VkImageView imageView_;
};

class VulkanSampler {
public:
    static constexpr VkObjectType  ObjectType = VK_OBJECT_TYPE_SAMPLER;

    const VkSamplerCreateInfo spec;

    VulkanSampler(VkDevice device, VkSampler sampler, const VkSamplerCreateInfo& spec);

    ~VulkanSampler();

    operator VkSampler() const;

public:
    VkDevice device_;
    VkSampler sampler_;
};

using VulkanImagePtr = std::unique_ptr<VulkanImage>;
using VulkanImageSptr = std::shared_ptr<VulkanImage>;
using VulkanImageViewPtr = std::unique_ptr<VulkanImageView>;
using VulkanImageViewSptr = std::shared_ptr<VulkanImageView>;
using VulkanSamplerPtr = std::unique_ptr<VulkanSampler>;
using VulkanSamplerSptr = std::shared_ptr<VulkanSampler>;

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

    [[nodiscard]] const VkImageCreateInfo& info();

    VulkanImagePtr make_unique();

    VulkanImageSptr make_shared();

private:
    VkDevice device_{};
    VmaAllocator allocator_{};
    VkImageCreateInfo info_{ };
};

class VulkanImageViewCreator{
public:
    explicit VulkanImageViewCreator(VkDevice device);

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

    [[nodiscard]] const VkImageViewCreateInfo& info() const;

    VkImageView create();

    VulkanImageViewPtr make_unique();

    VulkanImageViewSptr make_shared();

private:
    VkDevice device_{};
    VkImageViewCreateInfo info_;
};

class VulkanSamplerCreator {
public:
    explicit VulkanSamplerCreator(VkDevice device);

    VulkanSamplerCreator& flags(VkSamplerCreateFlags flags);

    VulkanSamplerCreator& magFilter(VkFilter value);

    VulkanSamplerCreator& minFilter(VkFilter value);

    VulkanSamplerCreator& mipmapMode(VkSamplerMipmapMode mode);

    VulkanSamplerCreator& addressMode(VkSamplerAddressMode u, VkSamplerAddressMode v,
                                      VkSamplerAddressMode w = VK_SAMPLER_ADDRESS_MODE_REPEAT);

    VulkanSamplerCreator& mipLodBias(float bias);

    VulkanSamplerCreator& anisotropyEnable(bool enable);

    VulkanSamplerCreator& maxAnisotropy(float maxValue);

    VulkanSamplerCreator& compareEnable(bool enable);

    VulkanSamplerCreator& compareOp(VkCompareOp op);

    VulkanSamplerCreator& minLod(float value);

    VulkanSamplerCreator& maxLod(float value);

    VulkanSamplerCreator& borderColor(VkBorderColor color);

    VulkanSamplerCreator& unnormalizedCoordinates(bool value);

    VkSampler create();

    VulkanSamplerPtr make_unqiue();

    VulkanSamplerSptr make_shared();

    [[nodiscard]] const VkSamplerCreateInfo& info() const;

private:
    VkDevice device_;
    VkSamplerCreateInfo info_;
};