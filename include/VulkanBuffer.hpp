#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>

#include <vector>
#include <memory>

class VulkanBuffer {
public:
    const VkBufferCreateInfo spec;
    const VmaMemoryUsage  memoryUsage;

    VulkanBuffer(VmaAllocator allocator, VmaAllocation allocation, VkBuffer buffer, VkBufferCreateInfo _spec, VmaMemoryUsage usage);

    ~VulkanBuffer();

    void* map();

    void unmap();

    operator VkBuffer() const;

    operator VkBuffer*();

private:
    VmaAllocator _allocator;
    VmaAllocation _allocation;
    VkBuffer _buffer;
    void* _mapping{};
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
    VmaAllocator _allocator;
    std::vector<uint32_t> _queueFamilyIndexes;
    VkBufferCreateInfo _info { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    VmaMemoryUsage _memoryUsage{VMA_MEMORY_USAGE_AUTO};
};