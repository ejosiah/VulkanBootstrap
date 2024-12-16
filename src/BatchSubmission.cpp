#include "Types.hpp"
#include "BatchSubmission.hpp"

BatchSubmission::BatchSubmission(VkQueue queue)
: queue_(queue){}

void BatchSubmission::enqueue(std::span<VkCommandBuffer> commandBuffers) {
    commandBuffers_.insert(commandBuffers_.end(), commandBuffers.begin(), commandBuffers.end());
}

void BatchSubmission::enqueue(VkCommandBuffer commandBuffer) {
    commandBuffers_.push_back(commandBuffer);
}

void BatchSubmission::enqueueSignal(VkSemaphore semaphore) {
    signals_.push_back(semaphore);
}

void BatchSubmission::enqueueWait(VkSemaphore semaphore, VkPipelineStageFlags flag) {
    waits_.push_back(semaphore);
    waitFlags_.push_back(flag);
}

void BatchSubmission::waitIdle() const {
    vkQueueWaitIdle(queue_);
}

VkResult BatchSubmission::execute(VkFence fence) {
    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = to<uint32>(waits_.size()),
        .pWaitSemaphores = waits_.data(),
        .pWaitDstStageMask = waitFlags_.data(),
        .commandBufferCount = to<uint32>(commandBuffers_.size()),
        .pCommandBuffers = commandBuffers_.data(),
        .signalSemaphoreCount = to<uint32>(signals_.size()),
        .pSignalSemaphores = signals_.data()
    };

    auto result = vkQueueSubmit(queue_, 1, &submitInfo, fence);

    commandBuffers_.clear();
    signals_.clear();
    waits_.clear();
    waitFlags_.clear();
    return result;
}

BatchSubmission &BatchSubmission::instance(VkQueue queue) {
    static BatchSubmission batchSubmission{queue};
    return batchSubmission;
}
