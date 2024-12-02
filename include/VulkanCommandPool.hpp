#pragma once

#include "volk.h"

#include <memory>
#include <vector>
#include <functional>

class VulkanCommandPool {
    using Body = std::function<void(VkCommandBuffer)>;

public:
    VulkanCommandPool(VkDevice device, VkQueue queue, VkCommandPool commandPool);

    ~VulkanCommandPool();

    [[nodiscard]] VkCommandBuffer allocateOne(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;

    std::vector<VkCommandBuffer> allocate(uint32_t count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    void oneTime(Body&& body);

    static std::shared_ptr<VulkanCommandPool> make_shared(VkDevice device, VkQueue queue, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags);

private:
    VkDevice _device;
    VkQueue _queue;
    VkCommandPool _commandPool;
};