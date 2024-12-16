#include "Types.hpp"
#include "VulkanRenderer.hpp"
#include "event/Events.hpp"
#include "WindowInterface.hpp"
#include "AppState.hpp"
#include "util/Bits.hpp"

#include <spdlog/spdlog.h>

#include <stdexcept>
#include <utility>

VulkanRenderer::VulkanRenderer(
        std::shared_ptr<Window> window,
        std::shared_ptr<VulkanInstance> instance,
        std::shared_ptr<VulkanDevice> device,
        std::unique_ptr<VulkanSwapchain> swapchain,
        VulkanSwapchainBuilder swapchainBuilder,
        BatchSubmission& batchSubmission,
        VkSampleCountFlagBits samples)
        : window_(std::move(window))
        , instance_(std::move(instance))
        , device_(std::move(device))
        , swapchain_(std::move(swapchain))
        , swapchainBuilder_(std::move(swapchainBuilder))
        , batchSubmission_(batchSubmission)
        , samples_(ensureSampleCount(samples, device_))
        {}

void VulkanRenderer::init() {
    createCommandPool();
    createSynchronizationPrimitives();
    initFrameBufferPrimitives();
}

void VulkanRenderer::renderFrame(VkCommandBuffer commandBuffer) {
    vkWaitForFences(*device_, 1, &inFlightFrame_[currentFrame_], false, UINT64_MAX);

    uint32 imageIndex = ~0u;
    auto status = vkAcquireNextImageKHR(*device_, *swapchain_, UINT64_MAX, acquireImageSemaphore_[currentFrame_], VK_NULL_HANDLE, &imageIndex);
    if(status == VK_SUBOPTIMAL_KHR || status == VK_ERROR_OUT_OF_DATE_KHR){
        EventBus::Publish(Events::Invalidate);
        return;
    }

    vkResetFences(*device_, 1, &inFlightFrame_[currentFrame_]);

    depthBuffer_.attachment.imageView = *depthBuffer_.imageView[imageIndex];
    renderingInfo_.pColorAttachments = &colorBuffer_.attachment[imageIndex];
    recordScene(commandBuffer);

    batchSubmission_.enqueue(renderCommandBuffer_[currentFrame_]);
    batchSubmission_.enqueueWait(acquireImageSemaphore_[currentFrame_], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    batchSubmission_.enqueueSignal(renderingFinishedSemaphore_[currentFrame_]);
    batchSubmission_.execute(inFlightFrame_[currentFrame_]);

    presentInfo_.pWaitSemaphores = &renderingFinishedSemaphore_[currentFrame_];
    presentInfo_.pImageIndices = &imageIndex;
    status = vkQueuePresentKHR(device_->getPresentQueue(), &presentInfo_);
    if(status == VK_SUBOPTIMAL_KHR || status == VK_ERROR_OUT_OF_DATE_KHR){
        EventBus::Publish(Events::Invalidate);
        return;
    }
    currentFrame_ = (currentFrame_ + 1 ) % MAX_IN_FLIGHT_FRAMES;
}

void VulkanRenderer::createCommandPool() {
    commandPool_ = device_->createCommandPool(VK_QUEUE_GRAPHICS_BIT);
    renderCommandBuffer_ = commandPool_->allocate(MAX_IN_FLIGHT_FRAMES);
}

void VulkanRenderer::createSynchronizationPrimitives() {
    VkSemaphoreCreateInfo semaphoreCreateInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fenceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for(auto i = 0; i < MAX_IN_FLIGHT_FRAMES; ++i) {
        auto result = vkCreateSemaphore(device_->handle(), &semaphoreCreateInfo, nullptr, &acquireImageSemaphore_[i]);
        if (result != VK_SUCCESS) {
            throw std::runtime_error{"unable to create acquireImageSemaphore"};
        }
        result = vkCreateSemaphore(device_->handle(), &semaphoreCreateInfo, nullptr, &renderingFinishedSemaphore_[i]);
        if (result != VK_SUCCESS) {
            throw std::runtime_error{"unable to create renderingFinishedSemaphore"};
        }


        result = vkCreateFence(device_->handle(), &fenceCreateInfo, nullptr, &inFlightFrame_[i]);
        if (result != VK_SUCCESS) {
            throw std::runtime_error{"unable to create inFlightFrame_ fence"};
        }
    }

    presentInfo_ = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    presentInfo_.waitSemaphoreCount = 1;
    presentInfo_.swapchainCount = 1;
    presentInfo_.pSwapchains = *swapchain_;
}

void VulkanRenderer::destroySynchronizationPrimitives() {

    for(auto i = 0; i < MAX_IN_FLIGHT_FRAMES; ++i) {
        vkDestroySemaphore(device_->handle(), acquireImageSemaphore_[i], VK_NULL_HANDLE);
        vkDestroySemaphore(device_->handle(), renderingFinishedSemaphore_[i], VK_NULL_HANDLE);
        vkDestroyFence(device_->handle(), inFlightFrame_[i], VK_NULL_HANDLE);
    }
}

void VulkanRenderer::initFrameBufferPrimitives() {
    if(samples_ != VK_SAMPLE_COUNT_1_BIT){
        colorBuffer_.msaaImages.clear();
        colorBuffer_.msaaImages.resize(swapchain_->imageCount());
    }

    std::vector<VkImageMemoryBarrier2> barriers;
    for(auto i = 0; i < swapchain_->imageCount(); ++i){
        if(samples_ != VK_SAMPLE_COUNT_1_BIT){
            colorBuffer_.msaaImages[i] =
                device_->image()
                    .imageType(VK_IMAGE_TYPE_2D)
                    .format(swapchain_->format())
                    .width(swapchain_->width())
                    .height(swapchain_->height())
                    .usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
                    .samples(samples_)
                .make_shared();
        }

        auto image = (samples_ == VK_SAMPLE_COUNT_1_BIT) ? swapchain_->getImage(i) : *colorBuffer_.msaaImages[i];
        colorBuffer_._[i].image = image;
        colorBuffer_._[i].imageView =
            device_->imageView()
                    .image(image)
                    .format(swapchain_->format())
                .make_unique();

        if(samples_ != VK_SAMPLE_COUNT_1_BIT) {
            colorBuffer_._[i].resolveImageView =
                device_->imageView()
                    .image(swapchain_->getImage(i))
                    .format(swapchain_->format())
                .make_unique();
        }

        VkImageMemoryBarrier2 barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_HOST_BIT,
            .srcAccessMask = VK_ACCESS_NONE,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .image = swapchain_->getImage(i),
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
        };

        barriers.push_back(barrier);

        if(samples_ != VK_SAMPLE_COUNT_1_BIT) {
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.image = *colorBuffer_.msaaImages[i];
            barriers.push_back(barrier);
        }

    }

    VkDependencyInfo info{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = to<uint32>(barriers.size()),
            .pImageMemoryBarriers = barriers.data()
    };

    commandPool_->oneTime([&](auto commandBuffer){
        vkCmdPipelineBarrier2(commandBuffer, &info);
    });


    depthBuffer_.image =
        device_->image()
            .imageType(VK_IMAGE_TYPE_2D)
            .format(VK_FORMAT_D32_SFLOAT) // // TODO query this from device using vkGetPhysicalDeviceFormatProperties
            .width(swapchain_->width())
            .height(swapchain_->height())
            .usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            .samples(samples_)
        .make_unique();

    depthBuffer_.imageView.resize(swapchain_->imageCount());
    for(auto i = 0; i < swapchain_->imageCount(); ++i){
        depthBuffer_.imageView[i] =
            device_->imageView()
                    .image(*depthBuffer_.image)
                    .format(VK_FORMAT_D32_SFLOAT)
                    .aspectMask(VK_IMAGE_ASPECT_DEPTH_BIT)
                .make_unique();
    }

    VkImageMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    barrier.srcStageMask = VK_PIPELINE_STAGE_HOST_BIT;
    barrier.srcAccessMask = VK_ACCESS_NONE;
    barrier.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    barrier.image = *depthBuffer_.image;
    barrier.subresourceRange = depthBuffer_.imageView.front()->spec.subresourceRange;

    info.imageMemoryBarrierCount = 1;
    info.pImageMemoryBarriers = &barrier;

    commandPool_->oneTime([&](auto commandBuffer){
        vkCmdPipelineBarrier2(commandBuffer, &info);
    });

    colorBuffer_.attachment.resize(swapchain_->imageCount());
    for(auto i = 0; i < swapchain_->imageCount(); ++i){
        VkRenderingAttachmentInfo attachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.imageView = *colorBuffer_._[i].imageView;
        attachment.clearValue.color = clearColor_;
        if(samples_ != VK_SAMPLE_COUNT_1_BIT){
            attachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
            attachment.resolveImageView = *colorBuffer_._[i].resolveImageView;
            attachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        colorBuffer_.attachment[i] = attachment;
    }


    depthBuffer_.attachment = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    depthBuffer_.attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthBuffer_.attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthBuffer_.attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthBuffer_.attachment.clearValue.depthStencil = {1.f, 0u};

    renderingInfo_.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    renderingInfo_.renderArea = { {0, 0}, { swapchain_->width(), swapchain_->height() }};
    renderingInfo_.layerCount = 1;
    renderingInfo_.colorAttachmentCount = 1;
    renderingInfo_.pDepthAttachment = &depthBuffer_.attachment;

}

void VulkanRenderer::recordScene(VkCommandBuffer sceneCommandBuffer) {
    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    vkBeginCommandBuffer(renderCommandBuffer_[currentFrame_], &beginInfo);
    vkCmdBeginRendering(renderCommandBuffer_[currentFrame_], &renderingInfo_);

    vkCmdExecuteCommands(renderCommandBuffer_[currentFrame_], 1, &sceneCommandBuffer);

    vkCmdEndRendering(renderCommandBuffer_[currentFrame_]);
    vkEndCommandBuffer(renderCommandBuffer_[currentFrame_]);
}

void VulkanRenderer::clearColor(float r, float g, float b, float a) {
    clearColor_ = {r, g, b, a};
    for(auto& attachment : colorBuffer_.attachment){
        attachment.clearValue.color = clearColor_;
    }
}

void VulkanRenderer::stop() {
    destroySynchronizationPrimitives();
}

void VulkanRenderer::invalidateSwapchain() {
    window_->aWaitEvents();

    device_->wait();
    destroySynchronizationPrimitives();

    auto [width, height] = window_->dimensions();
    swapchainBuilder_.setExtent(width, height);
    swapchain_ = swapchainBuilder_.make_unique();
    init();
}

uint32 VulkanRenderer::width() const {
    return swapchain_->width();
}

uint32 VulkanRenderer::height() const {
    return swapchain_->height();
}

VkSampleCountFlagBits VulkanRenderer::samples() const {
    return samples_;
}

VkFormat VulkanRenderer::format() const {
    return swapchain_->format();
}

VkFormat VulkanRenderer::depthFormat() const {
    return VK_FORMAT_D32_SFLOAT;
}

uint32 VulkanRenderer::framesInFlight() const {
    return MAX_IN_FLIGHT_FRAMES;
}

uint32 VulkanRenderer::colorBufferCount() const {
    return swapchain_->imageCount();
}

VkSampleCountFlagBits
VulkanRenderer::ensureSampleCount(VkSampleCountFlagBits samples, const std::shared_ptr<VulkanDevice>& device) {
    if(samples == VK_SAMPLE_COUNT_1_BIT) return samples;

    auto limits = device->getProperties().limits;
    auto supportedSamples = limits.framebufferColorSampleCounts & limits.framebufferDepthSampleCounts;
    auto maxSupportedSamples =  to<VkSampleCountFlagBits>(bits::findHighestBitSet(supportedSamples));

    if(samples & maxSupportedSamples){
        return samples;
    }
    return maxSupportedSamples;
}

uint32 VulkanRenderer::currentFrame() const {
    return currentFrame_;
}

bool VulkanRenderer::msaaEnabled() const {
    return !colorBuffer_.msaaImages.empty();
}
