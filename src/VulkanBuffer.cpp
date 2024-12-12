#include "Types.hpp"
#include "VulkanBuffer.hpp"
#include "SetVulkanObjectName.hpp"

VulkanBuffer::VulkanBuffer(VmaAllocator allocator, VmaAllocation allocation, VkBuffer buffer, VkBufferCreateInfo aSpec, VmaAllocationCreateInfo allocSpec)
: allocator_(allocator)
, allocation_(allocation)
, buffer_(buffer)
, spec(aSpec)
, allocationSpec(allocSpec){}

VulkanBuffer::~VulkanBuffer() {
    if(mapping_){
        unmap();
    }
    vmaDestroyBuffer(allocator_, buffer_, allocation_);
}

uint8 *VulkanBuffer::map() {
    if(allocationSpec.usage == VMA_MEMORY_USAGE_GPU_ONLY){
        return nullptr;
    };
    vmaMapMemory(allocator_, allocation_, &mapping_);
    return reinterpret_cast<uint8*>(mapping_);
}

void VulkanBuffer::unmap() {
    vmaUnmapMemory(allocator_, allocation_);
}

VulkanBuffer::operator VkBuffer() const {
    return buffer_;
}

VulkanBuffer::operator VkBuffer *() {
    return &buffer_;
}

VulkanBufferCreator::VulkanBufferCreator(VkDevice device, VmaAllocator allocator)
: device_(device)
, allocator_(allocator) {}

VulkanBufferCreator &VulkanBufferCreator::flags(VkBufferCreateFlags flags) {
    info_.flags = flags;
    return *this;
}

VulkanBufferCreator &VulkanBufferCreator::size(VkDeviceSize size) {
    info_.size = size;
    return *this;
}

VulkanBufferCreator &VulkanBufferCreator::usage(VkBufferUsageFlags usageFlags) {
    info_.usage = usageFlags;
    return *this;
}

VulkanBufferCreator &VulkanBufferCreator::memoryUsage(VmaMemoryUsage memoryUsage) {
    memoryUsage_ = memoryUsage;
    return *this;
}

VulkanBufferCreator &VulkanBufferCreator::sharingMode(VkSharingMode sharingMode) {
    info_.sharingMode = sharingMode;
    return *this;
}

VulkanBufferCreator &VulkanBufferCreator::addQueueFamilyIndex(uint32_t queueFamilyIndex) {
    queueFamilyIndexes_.push_back(queueFamilyIndex);
    return *this;
}

std::tuple<VkBuffer, VmaAllocation, VmaAllocationCreateInfo> VulkanBufferCreator::create() {
    info_.queueFamilyIndexCount = to<uint32>(queueFamilyIndexes_.size());
    info_.pQueueFamilyIndices = queueFamilyIndexes_.data();

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = memoryUsage_;

    VkBuffer buffer;
    VmaAllocation allocation;
    vmaCreateBuffer(allocator_, &info_, &allocInfo, &buffer, &allocation, nullptr);

    if(!name_.empty()){
        setVulkanObjectName(device_, VK_OBJECT_TYPE_BUFFER, buffer, name_);
    }
    return std::make_tuple(buffer, allocation, allocInfo);
}

std::shared_ptr<VulkanBuffer> VulkanBufferCreator::make_shared() {
    auto [buffer, allocation, allocInfo] = create();
    return std::make_shared<VulkanBuffer>(allocator_, allocation, buffer, info_, allocInfo);
}

VulkanBufferCreator &VulkanBufferCreator::name(std::string_view name) {
    name_ = name;
    return *this;
}

std::unique_ptr<VulkanBuffer> VulkanBufferCreator::make_unique() {
    auto [buffer, allocation, allocInfo] = create();
    return std::make_unique<VulkanBuffer>(allocator_, allocation, buffer, info_, allocInfo);
}
