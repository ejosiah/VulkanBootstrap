#include "VulkanCommandPool.hpp"

VulkanCommandPool::VulkanCommandPool(VkDevice device, VkQueue queue, VkCommandPool commandPool)
: _device(device)
, _queue(queue)
, _commandPool(commandPool){}

VulkanCommandPool::~VulkanCommandPool() {
    vkDestroyCommandPool(_device, _commandPool, nullptr);
}

VkCommandBuffer VulkanCommandPool::allocate() const {
    VkCommandBufferAllocateInfo info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    info.commandPool = _commandPool;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = 1;

    VkCommandBuffer commandBuffer{};
    vkAllocateCommandBuffers(_device, &info, &commandBuffer);

    return commandBuffer;
}

std::vector<VkCommandBuffer> VulkanCommandPool::allocate(uint32_t count) {
    VkCommandBufferAllocateInfo info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    info.commandPool = _commandPool;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = count;

    std::vector<VkCommandBuffer> commandBuffers(count);
    vkAllocateCommandBuffers(_device, &info, commandBuffers.data());

    return commandBuffers;
}

void VulkanCommandPool::oneTime(auto body) {
    auto commandBuffer = allocate();

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

    vkQueueSubmit(_queue, 1, &submitInfo, nullptr);

    vkQueueWaitIdle(_queue);
}

std::shared_ptr<VulkanCommandPool>
VulkanCommandPool::make_shared(VkDevice device, VkQueue queue, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags) {
    VkCommandPoolCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = flags,
        .queueFamilyIndex = queueFamilyIndex
    };

    VkCommandPool commandPool;
    vkCreateCommandPool(device, &createInfo, nullptr, &commandPool);
    return std::make_shared<VulkanCommandPool>(device, queue, commandPool);
}
