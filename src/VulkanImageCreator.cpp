#include "Types.hpp"
#include "VulkanImage.hpp"
#include "vk_mem_alloc.h"
#include <cpptrace/cpptrace.hpp>

#include <stdexcept>

VulkanImageCreator::VulkanImageCreator(VkDevice device, VmaAllocator allocator)
: device_(device)
, allocator_(allocator)
, info_{
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
    info_.flags = flags;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::imageType(VkImageType imageType) {
    info_.imageType = imageType;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::format(VkFormat format) {
    info_.format = format;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::extent(VkExtent3D extent) {
    info_.extent = extent;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::width(uint32 width) {
    info_.extent.width = width;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::height(uint32 height) {
    info_.extent.height = height;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::depth(uint32 depth) {
    info_.extent.depth = depth;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::mipLevels(int levels) {
    info_.mipLevels = levels;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::arrayLayers(int layers) {
    info_.arrayLayers =  layers;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::samples(VkSampleCountFlagBits samples) {
    info_.samples = samples;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::tiling(VkImageTiling tiling) {
    info_.tiling = tiling;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::usage(VkImageUsageFlagBits usage) {
    info_.usage |= usage;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::sharingMode(VkSharingMode mode) {
    info_.sharingMode = mode;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::queueFamilyIndexCount(uint32 count) {
    info_.queueFamilyIndexCount = count;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::pQueueFamilyIndices(const uint32 *ptr) {
    info_.pQueueFamilyIndices = ptr;
    return *this;
}

VulkanImageCreator &VulkanImageCreator::initialLayout(VkImageLayout layout) {
    info_.initialLayout = layout;
    return *this;
}

std::tuple<VkImage, VmaAllocation> VulkanImageCreator::create() {
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VkImage image;
    VmaAllocation allocation;
    auto result = vmaCreateImage(allocator_, &info_, &allocInfo, &image, &allocation, nullptr);

    if(result != VK_SUCCESS) {
        throw std::runtime_error{ "unable to create image"};
    }
    return std::make_tuple(image, allocation);
}

std::unique_ptr<VulkanImage> VulkanImageCreator::make_unique() {
    auto [image, allocation] = create();
    return std::make_unique<VulkanImage>(allocator_, allocation, image, info_);
}

std::shared_ptr<VulkanImage> VulkanImageCreator::make_shared() {
    auto [image, allocation] = create();
    return std::make_shared<VulkanImage>(allocator_, allocation, image, info_);}

const VkImageCreateInfo &VulkanImageCreator::info() {
    return info_;
}

VulkanImageViewCreator::VulkanImageViewCreator(VkDevice device)
: device_(device)
, info_{
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .viewType = VK_IMAGE_VIEW_TYPE_2D,
    .components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
    .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
}
{}

VulkanImageViewCreator &VulkanImageViewCreator::flags(VkImageViewCreateFlagBits flags) {
    info_.flags = flags;
    return *this;
}

VulkanImageViewCreator &VulkanImageViewCreator::image(VkImage image) {
    info_.image = image;
    return *this;
}

VulkanImageViewCreator &VulkanImageViewCreator::viewType(VkImageViewType viewType) {
    info_.viewType = viewType;
    return *this;
}

VulkanImageViewCreator &VulkanImageViewCreator::format(VkFormat format) {
    info_.format = format;
    return *this;
}

VulkanImageViewCreator &VulkanImageViewCreator::componentSwizzleR(VkComponentSwizzle swizzle) {
    info_.components.r = swizzle;
    return *this;
}


VulkanImageViewCreator &VulkanImageViewCreator::componentSwizzleG(VkComponentSwizzle swizzle) {
    info_.components.g = swizzle;
    return *this;
}


VulkanImageViewCreator &VulkanImageViewCreator::componentSwizzleB(VkComponentSwizzle swizzle) {
    info_.components.b = swizzle;
    return *this;
}


VulkanImageViewCreator &VulkanImageViewCreator::componentSwizzleA(VkComponentSwizzle swizzle) {
    info_.components.a = swizzle;
    return *this;
}

VulkanImageViewCreator &VulkanImageViewCreator::aspectMask(VkImageAspectFlagBits aspectMask) {
    info_.subresourceRange.aspectMask = aspectMask;
    return *this;
}

VulkanImageViewCreator &VulkanImageViewCreator::baseMipLevel(uint32 level) {
    info_.subresourceRange.baseMipLevel = level;
    return *this;
}

VulkanImageViewCreator &VulkanImageViewCreator::levelCount(uint32 count) {
    info_.subresourceRange.levelCount = count;
    return *this;
}

VulkanImageViewCreator &VulkanImageViewCreator::baseArrayLayer(uint32 layer) {
    info_.subresourceRange.baseArrayLayer = layer;
    return *this;
}

VulkanImageViewCreator &VulkanImageViewCreator::layerCount(uint32 count) {
    info_.subresourceRange.layerCount = count;
    return *this;
}

VkImageView VulkanImageViewCreator::create() {
    if(!info_.image){
        throw std::runtime_error{ "image is not yet defined" };
    }

        if(info_.format == VK_FORMAT_UNDEFINED) {
            throw std::runtime_error{ "format is undefined" };
        }

    VkImageView imageView;
    auto result = vkCreateImageView(device_, &info_, VK_NULL_HANDLE, &imageView);

    if(result != VK_SUCCESS){
        throw std::runtime_error{ "unable to create image view"};
    }

    return imageView;
}

std::unique_ptr<VulkanImageView> VulkanImageViewCreator::make_unique() {
    return std::make_unique<VulkanImageView>(device_, create(), info_);
}

std::shared_ptr<VulkanImageView> VulkanImageViewCreator::make_shared() {
    return std::make_shared<VulkanImageView>(device_, create(), info_);
}

const VkImageViewCreateInfo& VulkanImageViewCreator::info() const {
    return info_;
}

VulkanImage::VulkanImage(VmaAllocator allocator, VmaAllocation allocation, VkImage image,
                         const VkImageCreateInfo &aSpec)
: allocator_(allocator)
, allocation_(allocation)
, image_(image)
, spec(aSpec){}

VulkanImage::~VulkanImage() {
    vmaDestroyImage(allocator_, image_, allocation_);
}

VulkanImage::operator VkImage() const {
    return image_;
}

VulkanImageView::VulkanImageView(VkDevice device, VkImageView imageView, const VkImageViewCreateInfo& aSpec)
: device_(device)
, imageView_(imageView)
, spec(aSpec){}

VulkanImageView::~VulkanImageView() {
    vkDestroyImageView(device_, imageView_, nullptr);
}

VulkanImageView::operator VkImageView() const {
    return imageView_;
}

VulkanSamplerCreator::VulkanSamplerCreator(VkDevice device)
: device_(device)
, info_{
   .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
   .magFilter = VK_FILTER_LINEAR,
   .minFilter = VK_FILTER_LINEAR,
   .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
   .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
   .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
   .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
   .minLod = 0,
   .maxLod = 1,
   .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK
}{}

VulkanSamplerCreator &VulkanSamplerCreator::flags(VkSamplerCreateFlags flags) {
    info_.flags = flags;
    return *this;
}

VulkanSamplerCreator &VulkanSamplerCreator::magFilter(VkFilter value) {
    info_.magFilter = value;
    return *this;
}


VulkanSamplerCreator &VulkanSamplerCreator::minFilter(VkFilter value) {
    info_.minFilter = value;
    return *this;
}

VulkanSamplerCreator &VulkanSamplerCreator::mipmapMode(VkSamplerMipmapMode mode) {
    info_.mipmapMode = mode;
    return *this;
}

VulkanSamplerCreator &
VulkanSamplerCreator::addressMode(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w) {
    info_.addressModeU = u;
    info_.addressModeV = v;
    info_.addressModeW = w;
    return *this;
}

VulkanSamplerCreator &VulkanSamplerCreator::mipLodBias(float bias) {
    info_.mipLodBias = bias;
    return *this;
}

VulkanSamplerCreator &VulkanSamplerCreator::anisotropyEnable(bool enable) {
    info_.anisotropyEnable = enable;
    return *this;
}

VulkanSamplerCreator &VulkanSamplerCreator::maxAnisotropy(float maxValue) {
    info_.maxAnisotropy = maxValue;
    return *this;
}

VulkanSamplerCreator &VulkanSamplerCreator::compareEnable(bool enable) {
    info_.compareEnable = enable;
    return *this;
}

VulkanSamplerCreator &VulkanSamplerCreator::compareOp(VkCompareOp op) {
    info_.compareOp = op;
    return *this;
}

VulkanSamplerCreator &VulkanSamplerCreator::minLod(float value) {
    info_.minLod = value;
    return *this;
}


VulkanSamplerCreator &VulkanSamplerCreator::maxLod(float value) {
    info_.maxLod = value;
    return *this;
}

VulkanSamplerCreator &VulkanSamplerCreator::borderColor(VkBorderColor color) {
    info_.borderColor = color;
    return *this;
}

VulkanSamplerCreator &VulkanSamplerCreator::unnormalizedCoordinates(bool value) {
    info_.unnormalizedCoordinates = value;
    return *this;
}

VkSampler VulkanSamplerCreator::create() {
    VkSampler sampler{};
    auto status = vkCreateSampler(device_, &info_, nullptr, &sampler);

    if(status != VK_SUCCESS) {
        throw cpptrace::runtime_error{ "unable to create sampler" };
    }

    return sampler;
}

VulkanSamplerPtr VulkanSamplerCreator::make_unqiue() {
    return std::make_unique<VulkanSampler>(device_, create(), info_);
}

VulkanSamplerSptr VulkanSamplerCreator::make_shared() {
    return std::make_shared<VulkanSampler>(device_, create(), info_);
}

const VkSamplerCreateInfo &VulkanSamplerCreator::info() const {
    return info_;
}

VulkanSampler::VulkanSampler(VkDevice device, VkSampler sampler, const VkSamplerCreateInfo &spec)
: device_(device)
, sampler_(sampler)
, spec(spec){}

VulkanSampler::~VulkanSampler(){
    vkDestroySampler(device_, sampler_, nullptr);
}

VulkanSampler::operator VkSampler() const {
    return sampler_;
}