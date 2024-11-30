#pragma once

#include "volk.h"

#include <memory>
#include <vector>

class VulkanCommandPool {
public:
    VulkanCommandPool(VkDevice device, VkQueue queue, VkCommandPool commandPool);

    ~VulkanCommandPool();

    [[nodiscard]] VkCommandBuffer allocate() const;

    std::vector<VkCommandBuffer> allocate(uint32_t count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    void oneTime(auto body);

    static std::shared_ptr<VulkanCommandPool> make_shared(VkDevice device, VkQueue queue, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags);

private:
    VkDevice _device;
    VkQueue _queue;
    VkCommandPool _commandPool;
};