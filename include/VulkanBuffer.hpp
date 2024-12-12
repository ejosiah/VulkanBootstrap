#pragma once

#include "Types.hpp"
#include <volk.h>
#include <vk_mem_alloc.h>

#include <string_view>
#include <vector>
#include <memory>

class VulkanBuffer {
public:
    static constexpr VkObjectType  ObjectType = VK_OBJECT_TYPE_BUFFER;
    const VkBufferCreateInfo spec;
    const VmaAllocationCreateInfo  allocationSpec;

    VulkanBuffer(VmaAllocator allocator, VmaAllocation allocation, VkBuffer buffer,
                 VkBufferCreateInfo spec, VmaAllocationCreateInfo allocSpec);

    ~VulkanBuffer();

    uint8* map();

    void unmap();

    operator VkBuffer() const;

    operator VkBuffer*();

private:
    VmaAllocator allocator_;
    VmaAllocation allocation_;
    VkBuffer buffer_;
    void* mapping_{};
};

class VulkanBufferCreator {
public:
    VulkanBufferCreator(VkDevice device, VmaAllocator allocator);

    VulkanBufferCreator& name(std::string_view name);

    VulkanBufferCreator& flags(VkBufferCreateFlags flags);

    VulkanBufferCreator& size(VkDeviceSize size);

    VulkanBufferCreator& usage(VkBufferUsageFlags usageFlags);

    VulkanBufferCreator& memoryUsage(VmaMemoryUsage memoryUsage);

    VulkanBufferCreator& sharingMode(VkSharingMode sharingMode);

    VulkanBufferCreator& addQueueFamilyIndex(uint32_t queueFamilyIndex);

    std::tuple<VkBuffer, VmaAllocation, VmaAllocationCreateInfo> create();

    std::unique_ptr<VulkanBuffer> make_unique();

    std::shared_ptr<VulkanBuffer> make_shared();

private:
    VkDevice device_;
    VmaAllocator allocator_;
    std::vector<uint32_t> queueFamilyIndexes_;
    VkBufferCreateInfo info_ { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    VmaMemoryUsage memoryUsage_{VMA_MEMORY_USAGE_AUTO};
    std::string_view name_;
};