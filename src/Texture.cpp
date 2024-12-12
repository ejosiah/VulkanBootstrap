#include "Texture.hpp"
#include "io/IO.hpp"
#include "Converters.hpp"

#include <ranges>
#include <functional>

TextureBuilder::TextureBuilder(VulkanDevice* device)
: device_(device)
, imageCreator_(std::make_unique<TextureImageCreator>(this))
, imageViewCreator_(std::make_unique<TextureImageViewCreator>(this))
, samplerCreator_(std::make_unique<TextureSamplerCreator>(this))
{}

TextureBuilder::TextureBuilder(TextureBuilder *parent)
: parent_(parent){}

TextureBuilder &TextureBuilder::addPath(const std::filesystem::path& path) {
    if(parent_){
        parent_->addPath(path);
    } else {
        paths_.push_back(path);
    }
    return *this;
}

TextureBuilder &TextureBuilder::finalLayout(VkImageLayout layout, VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask) {
    if(parent_) {
        parent_->finalLayout(layout, dstStageMask, dstAccessMask);
    }else {
        finalLayout_ = layout;
        dstAccessMask_ = dstAccessMask;
        dstStageMask_ = dstStageMask;
    }
    return *this;
}

Texture TextureBuilder::create() {
    if(parent_){
        return parent_->create();
    }else {
        Texture texture{};
        VkImageMemoryBarrier2 barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
        VkDependencyInfo dependencyInfo{VK_STRUCTURE_TYPE_DEPENDENCY_INFO};

        const auto format = imageCreator_->info().format;
        const auto imageType = imageCreator_->info().imageType;
        const auto viewType = imageViewCreator_->info().viewType;
        const auto isCubeArray = viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        const auto isArray = viewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY || isCubeArray;
        const auto channels = format::toNumChannels(format);
        auto layerCount = imageCreator_->info().arrayLayers;

        if (!paths_.empty()) {
            using namespace std::placeholders;
            const auto images = paths_ | std::views::transform(std::bind(&io::loadImage, _1, channels, true));
            const auto imageCount = images.size();
            const auto imageSize = images.front().size;

            auto staging =
                device_->buffer()
                        .usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
                        .memoryUsage(VMA_MEMORY_USAGE_CPU_ONLY)
                        .size(images.size() * images.front().size)
                    .make_unique();


            uint32 width = images.front().width;
            uint32 height = images.front().height;
            uint32 depth = imageCreator_->info().extent.depth;

            imageCreator_->extent({width, height, depth});
            imageCreator_->arrayLayers(layerCount);
            imageCreator_->usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT);
            texture.image = imageCreator_->make_shared();

            std::vector<VkBufferImageCopy> copyRegions(images.size());
            auto mapping = staging->map();
            for (auto i = 0u; i < imageCount; ++i) {
                auto image = images[i];
                std::memcpy(mapping + (i * imageSize), image.data.data(), image.size);
                VkBufferImageCopy region{
                        .bufferOffset = i * imageSize,
                        .imageSubresource {
                                .aspectMask = imageViewCreator_->info().subresourceRange.aspectMask,
                                .mipLevel = 0,
                                .baseArrayLayer = isCubeArray ? i / 6 : i,
                                .layerCount = 1,
                        },
                        .imageExtent = {width, height, 1}
                };
                copyRegions[i] = region;
            }

            barrier = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                    .srcStageMask = VK_PIPELINE_STAGE_HOST_BIT,
                    .srcAccessMask = VK_ACCESS_NONE,
                    .dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
                    .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .image = *texture.image,
                    .subresourceRange{
                            .aspectMask = imageViewCreator_->info().subresourceRange.aspectMask,
                            .baseMipLevel = 0,
                            .levelCount = texture.image->spec.mipLevels,
                            .baseArrayLayer = 0,
                            .layerCount = layerCount
                    }
            };

            dependencyInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                    .imageMemoryBarrierCount = 1,
                    .pImageMemoryBarriers = &barrier
            };

            device_->graphicsCommandPool()->oneTime([&](auto commandBuffer) {
                vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);

                vkCmdCopyBufferToImage(commandBuffer, *staging, *texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                       copyRegions.size(), copyRegions.data());
            });

            barrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            device_->graphicsCommandPool()->oneTime([&](auto commandBuffer) {
                vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
            });
        }

        if (viewRequested_) {
            imageViewCreator_->format(imageCreator_->info().format);
            imageViewCreator_->setImage(*texture.image);
            texture.imageView = imageViewCreator_->make_shared();
        }
        if (samplerRequested_) {
            texture.sampler = samplerCreator_->make_shared();
        }

        if (finalLayout_ != VK_IMAGE_LAYOUT_UNDEFINED) {
            barrier = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                    .srcStageMask = VK_PIPELINE_STAGE_HOST_BIT,
                    .srcAccessMask = VK_ACCESS_NONE,
                    .dstStageMask = dstStageMask_,
                    .dstAccessMask = dstAccessMask_,
                    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .newLayout = finalLayout_,
                    .image = *texture.image,
                    .subresourceRange{
                            .aspectMask = imageViewCreator_->info().subresourceRange.aspectMask,
                            .baseMipLevel = 0,
                            .levelCount = texture.image->spec.mipLevels,
                            .baseArrayLayer = 0,
                            .layerCount = layerCount
                    }
            };

            dependencyInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                    .imageMemoryBarrierCount = 1,
                    .pImageMemoryBarriers = &barrier
            };

            device_->graphicsCommandPool()->oneTime([&](auto commandBuffer) {
                vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
            });
        }

        return texture;
    }
}

TextureImageCreator &TextureBuilder::image() {
    if(parent_){
        return parent_->image();
    } else {
        return *imageCreator_;
    }
}

TextureImageViewCreator &TextureBuilder::imageView() {
    if(parent_){
        return parent_->imageView();
    }else {
        viewRequested_ = true;
        return *imageViewCreator_;
    }
}

TextureSamplerCreator &TextureBuilder::sampler() {
    if(parent_){
        return parent_->sampler();
    }else {
        samplerRequested_ = true;
        return *samplerCreator_;
    }
}

TextureImageCreator::TextureImageCreator(TextureBuilder* parent)
: TextureBuilder(parent)
, imageCreator_(*parent->device_, parent->device_->allocator_)
{}

TextureImageCreator &TextureImageCreator::imageType(VkImageType imageType) {
    imageCreator_.imageType(imageType);
    return *this;
}

TextureImageCreator &TextureImageCreator::format(VkFormat format) {
    imageCreator_.format(format);
    return *this;
}

TextureImageCreator &TextureImageCreator::extent(VkExtent3D extent) {
    imageCreator_.extent(extent);
    return *this;
}

TextureImageCreator &TextureImageCreator::width(uint32_t width) {
    imageCreator_.width(width);
    return *this;
}

TextureImageCreator &TextureImageCreator::height(uint32_t height) {
    imageCreator_.height(height);
    return *this;
}

TextureImageCreator &TextureImageCreator::depth(uint32_t depth) {
    imageCreator_.depth(depth);
    return *this;
}

TextureImageCreator &TextureImageCreator::mipLevels(int levels) {
    imageCreator_.mipLevels(levels);
    return *this;
}

TextureImageCreator &TextureImageCreator::arrayLayers(int layers) {
    imageCreator_.arrayLayers(layers);
    return *this;
}

TextureImageCreator &TextureImageCreator::samples(VkSampleCountFlagBits samples) {
    imageCreator_.samples(samples);
    return *this;
}

TextureImageCreator &TextureImageCreator::tiling(VkImageTiling tiling) {
    imageCreator_.tiling(tiling);
    return *this;
}

TextureImageCreator &TextureImageCreator::usage(VkImageUsageFlagBits usage) {
    imageCreator_.usage(usage);
    return *this;
}

TextureImageCreator &TextureImageCreator::sharingMode(VkSharingMode mode) {
    imageCreator_.sharingMode(mode);
    return *this;
}

TextureImageCreator &TextureImageCreator::queueFamilyIndexCount(uint32_t count) {
    imageCreator_.queueFamilyIndexCount(count);
    return *this;
}

TextureImageCreator &TextureImageCreator::pQueueFamilyIndices(const uint32_t *ptr) {
    imageCreator_.pQueueFamilyIndices(ptr);
    return *this;
}

const VkImageCreateInfo &TextureImageCreator::info() {
    return imageCreator_.info();
}

VulkanImageSptr TextureImageCreator::make_shared() {
    return imageCreator_.make_shared();
}

TextureImageViewCreator::TextureImageViewCreator(TextureBuilder* parent)
: TextureBuilder(parent)
, imageViewCreator_(*parent->device_)
{}

TextureImageViewCreator &TextureImageViewCreator::viewType(VkImageViewType viewType) {
    imageViewCreator_.viewType(viewType);
    return *this;
}

TextureImageViewCreator &TextureImageViewCreator::aspectMask(VkImageAspectFlagBits aspectMask) {
    imageViewCreator_.aspectMask(aspectMask);
    return *this;
}

TextureImageViewCreator &TextureImageViewCreator::baseMipLevel(uint32_t level) {
    imageViewCreator_.baseMipLevel(0);
    return *this;
}

TextureImageViewCreator &TextureImageViewCreator::levelCount(uint32_t count) {
    imageViewCreator_.levelCount(count);
    return *this;
}

TextureImageViewCreator &TextureImageViewCreator::baseArrayLayer(uint32_t layer) {
    imageViewCreator_.baseArrayLayer(layer);
    return *this;
}

TextureImageViewCreator &TextureImageViewCreator::layerCount(uint32_t count) {
    imageViewCreator_.layerCount(count);
    return *this;
}

const VkImageViewCreateInfo &TextureImageViewCreator::info() {
    return imageViewCreator_.info();
}

TextureImageViewCreator &TextureImageViewCreator::format(VkFormat format) {
    imageViewCreator_.format(format);
    return *this;
}

TextureImageViewCreator &TextureImageViewCreator::setImage(VkImage image) {
    imageViewCreator_.image(image);
    return *this;
}

VulkanImageViewSptr TextureImageViewCreator::make_shared() {
    return imageViewCreator_.make_shared();
}

TextureSamplerCreator::TextureSamplerCreator(TextureBuilder *parent)
: TextureBuilder(parent)
, samplerCreator_(*parent->device_)
{}

TextureSamplerCreator &TextureSamplerCreator::flags(VkSamplerCreateFlags flags) {
    samplerCreator_.flags(flags);
    return *this;
}

TextureSamplerCreator &TextureSamplerCreator::magFilter(VkFilter value) {
    samplerCreator_.magFilter(value);
    return *this;
}


TextureSamplerCreator &TextureSamplerCreator::minFilter(VkFilter value) {
    samplerCreator_.minFilter(value);
    return *this;
}

TextureSamplerCreator &TextureSamplerCreator::mipmapMode(VkSamplerMipmapMode mode) {
    samplerCreator_.mipmapMode(mode);
    return *this;
}

TextureSamplerCreator &
TextureSamplerCreator::addressMode(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w) {
    samplerCreator_.addressMode(u, v, w);
    return *this;
}

TextureSamplerCreator &TextureSamplerCreator::mipLodBias(float bias) {
    samplerCreator_.mipLodBias(bias);
    return *this;
}

TextureSamplerCreator &TextureSamplerCreator::anisotropyEnable(bool enable) {
    samplerCreator_.anisotropyEnable(enable);
    return *this;
}

TextureSamplerCreator &TextureSamplerCreator::maxAnisotropy(float maxValue) {
    samplerCreator_.maxAnisotropy(maxValue);
    return *this;
}

TextureSamplerCreator &TextureSamplerCreator::compareEnable(bool enable) {
    samplerCreator_.compareEnable(enable);
    return *this;
}

TextureSamplerCreator &TextureSamplerCreator::compareOp(VkCompareOp op) {
    samplerCreator_.compareOp(op);
    return *this;
}

TextureSamplerCreator &TextureSamplerCreator::minLod(float value) {
    samplerCreator_.minLod(value);
    return *this;
}


TextureSamplerCreator &TextureSamplerCreator::maxLod(float value) {
    samplerCreator_.maxLod(value);
    return *this;
}

TextureSamplerCreator &TextureSamplerCreator::borderColor(VkBorderColor color) {
    samplerCreator_.borderColor(color);
    return *this;
}

TextureSamplerCreator &TextureSamplerCreator::unnormalizedCoordinates(bool value) {
    samplerCreator_.unnormalizedCoordinates(value);
    return *this;
}

VulkanSamplerSptr TextureSamplerCreator::make_shared() {
    return samplerCreator_.make_shared();
}
