#pragma once

#include "Types.hpp"
#include "VulkanDevice.hpp"
#include "VulkanImage.hpp"
#include <vector>
#include <string>
#include <filesystem>
#include <cpptrace/cpptrace.hpp>

struct TextureId { ResourceHandle id; };

struct Texture {
    static constexpr uint32 INVALID = ~0u;

    VulkanImageSptr image;
    VulkanImageViewSptr imageView;
    VulkanSamplerSptr sampler;
    std::string name;
};


class TextureBuilder {
public:
    friend class TextureImageCreator;
    friend class TextureImageViewCreator;
    friend class TextureSamplerCreator;

    explicit TextureBuilder(VulkanDevice* device = nullptr);

    explicit TextureBuilder(TextureBuilder* parent);

    TextureBuilder& addPath(const std::filesystem::path& path);

    TextureBuilder& finalLayout(VkImageLayout layout, VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask);

    TextureImageCreator& image();

    TextureImageViewCreator& imageView();

    TextureSamplerCreator& sampler();

    Texture create();

protected:
    TextureBuilder* parent_{};
    VulkanDevice* device_;
    std::unique_ptr<TextureImageCreator> imageCreator_;
    std::unique_ptr<TextureImageViewCreator> imageViewCreator_;
    std::unique_ptr<TextureSamplerCreator> samplerCreator_;
    std::vector<std::filesystem::path> paths_;
    VkImageLayout finalLayout_{VK_IMAGE_LAYOUT_UNDEFINED};
    VkPipelineStageFlags2 dstStageMask_{VK_PIPELINE_STAGE_2_HOST_BIT};
    VkAccessFlags2 dstAccessMask_{VK_ACCESS_2_NONE};
    bool viewRequested_{};
    bool samplerRequested_{};

};

class TextureImageCreator : public TextureBuilder {
public:
    explicit TextureImageCreator(TextureBuilder* parent);

    ~TextureImageCreator() = default;

    TextureImageCreator& imageType(VkImageType imageType);

    TextureImageCreator& format(VkFormat format);

    TextureImageCreator& extent(VkExtent3D extent);

    TextureImageCreator& width(uint32_t width);

    TextureImageCreator& height(uint32_t height);

    TextureImageCreator& depth(uint32_t depth);

    TextureImageCreator& mipLevels(int levels);

    TextureImageCreator& arrayLayers(int layers);

    TextureImageCreator& samples(VkSampleCountFlagBits samples);

    TextureImageCreator& tiling(VkImageTiling tiling);

    TextureImageCreator& usage(VkImageUsageFlagBits usage);

    TextureImageCreator& sharingMode(VkSharingMode mode);

    TextureImageCreator& queueFamilyIndexCount(uint32_t count);

    TextureImageCreator& pQueueFamilyIndices(const uint32_t* ptr);

    const VkImageCreateInfo& info();

    VulkanImageSptr make_shared();

private:
    VulkanImageCreator imageCreator_;
    TextureBuilder* parent_{};
};

class TextureImageViewCreator : public TextureBuilder {
public:
    explicit TextureImageViewCreator(TextureBuilder* parent);

    ~TextureImageViewCreator() = default;

    TextureImageViewCreator& viewType(VkImageViewType viewType);

    TextureImageViewCreator& aspectMask(VkImageAspectFlagBits aspectMask);

    TextureImageViewCreator& baseMipLevel(uint32_t level);

    TextureImageViewCreator& levelCount(uint32_t count);

    TextureImageViewCreator& baseArrayLayer(uint32_t layer);

    TextureImageViewCreator& layerCount(uint32_t count);

    TextureImageViewCreator& format(VkFormat format);

    TextureImageViewCreator& setImage(VkImage image);

    const VkImageViewCreateInfo& info();

    VulkanImageViewSptr make_shared();

private:
    VulkanImageViewCreator imageViewCreator_;
    TextureBuilder* parent_{};
};

class TextureSamplerCreator : public TextureBuilder {
public:
    explicit TextureSamplerCreator(TextureBuilder* parent);

    ~TextureSamplerCreator() = default;

    TextureSamplerCreator& flags(VkSamplerCreateFlags flags);

    TextureSamplerCreator& magFilter(VkFilter value);

    TextureSamplerCreator& minFilter(VkFilter value);

    TextureSamplerCreator& mipmapMode(VkSamplerMipmapMode mode);

    TextureSamplerCreator& addressMode(VkSamplerAddressMode u, VkSamplerAddressMode v,
                                      VkSamplerAddressMode w = VK_SAMPLER_ADDRESS_MODE_REPEAT);

    TextureSamplerCreator& mipLodBias(float bias);

    TextureSamplerCreator& anisotropyEnable(bool enable);

    TextureSamplerCreator& maxAnisotropy(float maxValue);

    TextureSamplerCreator& compareEnable(bool enable);

    TextureSamplerCreator& compareOp(VkCompareOp op);

    TextureSamplerCreator& minLod(float value);

    TextureSamplerCreator& maxLod(float value);

    TextureSamplerCreator& borderColor(VkBorderColor color);

    TextureSamplerCreator& unnormalizedCoordinates(bool value);

    VulkanSamplerSptr make_shared();

private:
    VulkanSamplerCreator samplerCreator_;
    TextureBuilder* parent_{};
};