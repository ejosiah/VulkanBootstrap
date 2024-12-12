#include "Types.hpp"
#include "VulkanCommandPool.hpp"

VulkanCommandPool::VulkanCommandPool(VkDevice device, VkQueue queue, VkCommandPool commandPool)
: device_(device)
, queue_(queue)
, commandPool_(commandPool){}

VulkanCommandPool::~VulkanCommandPool() {
    vkDestroyCommandPool(device_, commandPool_, nullptr);
}

VkCommandBuffer VulkanCommandPool::allocateOne(VkCommandBufferLevel level) const {
    VkCommandBufferAllocateInfo info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    info.commandPool = commandPool_;
    info.level = level;
    info.commandBufferCount = 1;

    VkCommandBuffer commandBuffer{};
    vkAllocateCommandBuffers(device_, &info, &commandBuffer);

    return commandBuffer;
}

std::vector<VkCommandBuffer> VulkanCommandPool::allocate(uint32 count, VkCommandBufferLevel level) {
    VkCommandBufferAllocateInfo info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    info.commandPool = commandPool_;
    info.level = level;
    info.commandBufferCount = count;

    std::vector<VkCommandBuffer> commandBuffers(count);
    vkAllocateCommandBuffers(device_, &info, commandBuffers.data());

    return commandBuffers;
}

void VulkanCommandPool::oneTime(Body&& body) {
    auto commandBuffer = allocateOne();

    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    body(commandBuffer);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue_, 1, &submitInfo, nullptr);

    vkQueueWaitIdle(queue_);
}

std::shared_ptr<VulkanCommandPool>
VulkanCommandPool::make_shared(VkDevice device, VkQueue queue, uint32 queueFamilyIndex, VkCommandPoolCreateFlags flags) {
    VkCommandPoolCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = flags,
        .queueFamilyIndex = queueFamilyIndex
    };

    VkCommandPool commandPool;
    vkCreateCommandPool(device, &createInfo, nullptr, &commandPool);
    return std::make_shared<VulkanCommandPool>(device, queue, commandPool);
}


std::unique_ptr<VulkanCommandPool>
VulkanCommandPool::make_unique(VkDevice device, VkQueue queue, uint32 queueFamilyIndex, VkCommandPoolCreateFlags flags) {
    VkCommandPoolCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = flags,
        .queueFamilyIndex = queueFamilyIndex
    };

    VkCommandPool commandPool;
    vkCreateCommandPool(device, &createInfo, nullptr, &commandPool);
    return std::make_unique<VulkanCommandPool>(device, queue, commandPool);
}

void VulkanCommandPool::reset() {
    vkResetCommandPool(device_, commandPool_, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT );
}


VkCommandPool VulkanCommandPool::handle() {
    return commandPool_;
}