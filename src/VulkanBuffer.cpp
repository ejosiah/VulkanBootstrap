#include "Types.hpp"
#include "VulkanBuffer.hpp"

VulkanBuffer::VulkanBuffer(VmaAllocator allocator, VmaAllocation allocation, VkBuffer buffer, VkBufferCreateInfo aSpec, VmaAllocationCreateInfo allocSpec)
: _allocator(allocator)
, _allocation(allocation)
, _buffer(buffer)
, spec(aSpec)
, allocationSpec(allocSpec){}

VulkanBuffer::~VulkanBuffer() {
    if(_mapping){
        unmap();
    }
    vmaDestroyBuffer(_allocator, _buffer, _allocation);
}

void *VulkanBuffer::map() {
    if(allocationSpec.usage == VMA_MEMORY_USAGE_GPU_ONLY){
        return nullptr;
    };
    vmaMapMemory(_allocator, _allocation, &_mapping);
    return _mapping;
}

void VulkanBuffer::unmap() {
    vmaUnmapMemory(_allocator, _allocation);
}

VulkanBuffer::operator VkBuffer() const {
    return _buffer;
}

VulkanBuffer::operator VkBuffer *() {
    return &_buffer;
}

VulkanBufferCreator::VulkanBufferCreator(VmaAllocator allocator)
: _allocator(allocator) {}

VulkanBufferCreator &VulkanBufferCreator::flags(VkBufferCreateFlags flags) {
    _info.flags = flags;
    return *this;
}

VulkanBufferCreator &VulkanBufferCreator::size(VkDeviceSize size) {
    _info.size = size;
    return *this;
}

VulkanBufferCreator &VulkanBufferCreator::usage(VkBufferUsageFlags usageFlags) {
    _info.usage = usageFlags;
    return *this;
}

VulkanBufferCreator &VulkanBufferCreator::memoryUsage(VmaMemoryUsage memoryUsage) {
    _memoryUsage = memoryUsage;
    return *this;
}

VulkanBufferCreator &VulkanBufferCreator::sharingMode(VkSharingMode sharingMode) {
    _info.sharingMode = sharingMode;
    return *this;
}

VulkanBufferCreator &VulkanBufferCreator::addQueueFamilyIndex(uint32_t queueFamilyIndex) {
    _queueFamilyIndexes.push_back(queueFamilyIndex);
    return *this;
}

std::shared_ptr<VulkanBuffer> VulkanBufferCreator::make_shared() {
    _info.queueFamilyIndexCount = to<uint32>(_queueFamilyIndexes.size());
    _info.pQueueFamilyIndices = _queueFamilyIndexes.data();

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = _memoryUsage;

    VkBuffer buffer;
    VmaAllocation allocation;
    vmaCreateBuffer(_allocator, &_info, &allocInfo, &buffer, &allocation, nullptr);

    return std::make_shared<VulkanBuffer>(_allocator, allocation, buffer, _info, allocInfo);
}
