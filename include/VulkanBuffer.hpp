#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>

#include <vector>
#include <memory>

class VulkanBuffer {
public:
    const VkBufferCreateInfo spec;
    const VmaAllocationCreateInfo  allocationSpec;

    VulkanBuffer(VmaAllocator allocator, VmaAllocation allocation, VkBuffer buffer,
                 VkBufferCreateInfo spec, VmaAllocationCreateInfo allocSpec);

    ~VulkanBuffer();

    void* map();

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
    VulkanBufferCreator(VmaAllocator allocator);

    VulkanBufferCreator& flags(VkBufferCreateFlags flags);

    VulkanBufferCreator& size(VkDeviceSize size);

    VulkanBufferCreator& usage(VkBufferUsageFlags usageFlags);

    VulkanBufferCreator& memoryUsage(VmaMemoryUsage memoryUsage);

    VulkanBufferCreator& sharingMode(VkSharingMode sharingMode);

    VulkanBufferCreator& addQueueFamilyIndex(uint32_t queueFamilyIndex);

    std::shared_ptr<VulkanBuffer> make_shared();

private:
    VmaAllocator allocator_;
    std::vector<uint32_t> queueFamilyIndexes_;
    VkBufferCreateInfo info_ { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    VmaMemoryUsage memoryUsage_{VMA_MEMORY_USAGE_AUTO};
};