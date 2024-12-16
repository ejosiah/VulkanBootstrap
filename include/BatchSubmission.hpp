#pragma once

#include <volk.h>
#include <span>
#include <vector>

class BatchSubmission {
public:
    BatchSubmission() = default;

    explicit BatchSubmission(VkQueue queue);

    void enqueue(std::span<VkCommandBuffer> commandBuffers);

    void enqueue(VkCommandBuffer commandBuffer);

    void enqueueSignal(VkSemaphore semaphore);

    void enqueueWait(VkSemaphore semaphore, VkPipelineStageFlags flag);

    VkResult execute(VkFence fence = VK_NULL_HANDLE);

    void waitIdle() const;

    static BatchSubmission& instance(VkQueue queue = VK_NULL_HANDLE);

private:
    VkQueue queue_{};
    std::vector<VkSemaphore> waits_;
    std::vector<VkPipelineStageFlags> waitFlags_;
    std::vector<VkSemaphore> signals_;
    std::vector<VkCommandBuffer> commandBuffers_;

};