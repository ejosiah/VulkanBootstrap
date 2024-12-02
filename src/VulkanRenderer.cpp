#include "Types.hpp"
#include "VulkanRenderer.hpp"
#include "event/Events.hpp"
#include "WindowInterface.hpp"

#include <spdlog/spdlog.h>

#include <stdexcept>
#include <utility>

VulkanRenderer::VulkanRenderer(
        std::shared_ptr<Window> window,
        std::shared_ptr<VulkanInstance> instance,
        std::shared_ptr<VulkanDevice> device,
        std::unique_ptr<VulkanSwapchain> swapchain,
        VulkanSwapchainBuilder swapchainBuilder)
        : _window(std::move(window))
        , _instance(std::move(instance))
        , _device(std::move(device))
        , _swapchain(std::move(swapchain))
        , _swapchainBuilder(std::move(swapchainBuilder)
        ){}

void VulkanRenderer::init() {
    createCommandPool();
    createSynchronizationPrimitives();
    initFrameBufferPrimitives();
}

void VulkanRenderer::renderFrame(VkCommandBuffer commandBuffer) {
    vkWaitForFences(*_device, 1, &_inFlightFrame[currentFrame], false, UINT64_MAX);

    uint32 imageIndex = ~0u;
    auto status = vkAcquireNextImageKHR(*_device, *_swapchain, UINT64_MAX, _acquireImageSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if(status == VK_SUBOPTIMAL_KHR || status == VK_ERROR_OUT_OF_DATE_KHR){
        EventBus::Publish(Events::Invalidate);
        return;
    }

    vkResetFences(*_device, 1, &_inFlightFrame[currentFrame]);

    _depthBuffer.attachment.imageView = *_depthBuffer.imageView[imageIndex];
    _renderingInfo.pColorAttachments = &_colorBuffer.attachment[imageIndex];
    recordScene(commandBuffer);

    _renderSubmitInfo.pCommandBuffers = &_renderCommandBuffer[currentFrame];
    _renderSubmitInfo.pWaitSemaphores = &_acquireImageSemaphore[currentFrame];
    _renderSubmitInfo.pSignalSemaphores = &_renderingFinishedSemaphore[currentFrame];
    vkQueueSubmit(_device->getGraphicsQueue(), 1, &_renderSubmitInfo, _inFlightFrame[currentFrame]);

    _presentInfo.pWaitSemaphores = &_renderingFinishedSemaphore[currentFrame];
    _presentInfo.pImageIndices = &imageIndex;
    status = vkQueuePresentKHR(_device->getPresentQueue(), &_presentInfo);
    if(status == VK_SUBOPTIMAL_KHR || status == VK_ERROR_OUT_OF_DATE_KHR){
        EventBus::Publish(Events::Invalidate);
        return;
    }
    currentFrame = (currentFrame + 1 ) % MAX_IN_FLIGHT_FRAMES;
}

void VulkanRenderer::createCommandPool() {
    _commandPool = _device->createCommandPool(VK_QUEUE_GRAPHICS_BIT);
    _renderCommandBuffer = _commandPool->allocate(MAX_IN_FLIGHT_FRAMES);
}

void VulkanRenderer::createSynchronizationPrimitives() {
    VkSemaphoreCreateInfo semaphoreCreateInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fenceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for(auto i = 0; i < MAX_IN_FLIGHT_FRAMES; ++i) {
        auto result = vkCreateSemaphore(_device->handle(), &semaphoreCreateInfo, nullptr, &_acquireImageSemaphore[i]);
        if (result != VK_SUCCESS) {
            throw std::runtime_error{"unable to create acquireImageSemaphore"};
        }
        result = vkCreateSemaphore(_device->handle(), &semaphoreCreateInfo, nullptr, &_renderingFinishedSemaphore[i]);
        if (result != VK_SUCCESS) {
            throw std::runtime_error{"unable to create renderingFinishedSemaphore"};
        }


        result = vkCreateFence(_device->handle(), &fenceCreateInfo, nullptr, &_inFlightFrame[i]);
        if (result != VK_SUCCESS) {
            throw std::runtime_error{"unable to create _inFlightFrame fence"};
        }
    }

    static VkPipelineStageFlags waitStage{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    _renderSubmitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
    _renderSubmitInfo.waitSemaphoreCount = 1;
    _renderSubmitInfo.pWaitDstStageMask = &waitStage;
    _renderSubmitInfo.commandBufferCount = 1;
    _renderSubmitInfo.signalSemaphoreCount = 1;

    _presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    _presentInfo.waitSemaphoreCount = 1;
    _presentInfo.swapchainCount = 1;
    _presentInfo.pSwapchains = *_swapchain;
}

void VulkanRenderer::destroySynchronizationPrimitives() {

    for(auto i = 0; i < MAX_IN_FLIGHT_FRAMES; ++i) {
        vkDestroySemaphore(_device->handle(), _acquireImageSemaphore[i], VK_NULL_HANDLE);
        vkDestroySemaphore(_device->handle(), _renderingFinishedSemaphore[i], VK_NULL_HANDLE);
        vkDestroyFence(_device->handle(), _inFlightFrame[i], VK_NULL_HANDLE);
    }
}

void VulkanRenderer::initFrameBufferPrimitives() {
    std::vector<VkImageMemoryBarrier> barriers(_swapchain->imageCount());
    for(auto i = 0; i < _swapchain->imageCount(); ++i){
        _colorBuffer._[i].image = _swapchain->getImage(i);
        _colorBuffer._[i].imageView =
            _device->imageView()
                    .image(_swapchain->getImage(i))
                    .format(_swapchain->format())
                .make_unique();

        barriers[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barriers[i].srcAccessMask = VK_ACCESS_NONE;
        barriers[i].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        barriers[i].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barriers[i].newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barriers[i].image = _swapchain->getImage(i);
        barriers[i].subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    }

    _commandPool->oneTime([&](auto commandBuffer){
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, barriers.size(), barriers.data());
    });


    _depthBuffer.image =
        _device->image()
            .imageType(VK_IMAGE_TYPE_2D)
            .format(VK_FORMAT_D32_SFLOAT) // // TODO query this from device using vkGetPhysicalDeviceFormatProperties
            .width(_swapchain->width())
            .height(_swapchain->height())
            .usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        .make_unique();

    _depthBuffer.imageView.resize(_swapchain->imageCount());
    for(auto i = 0; i < _swapchain->imageCount(); ++i){
        _depthBuffer.imageView[i] =
            _device->imageView()
                    .image(*_depthBuffer.image)
                    .format(VK_FORMAT_D32_SFLOAT)
                    .aspectMask(VK_IMAGE_ASPECT_DEPTH_BIT)
                .make_unique();
    }

    VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    barrier.srcAccessMask = VK_ACCESS_NONE;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    barrier.image = *_depthBuffer.image;
    barrier.subresourceRange = _depthBuffer.imageView.front()->spec.subresourceRange;

    _commandPool->oneTime([&](auto commandBuffer){
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    });

    _colorBuffer.attachment.resize(_swapchain->imageCount());
    for(auto i = 0; i < _swapchain->imageCount(); ++i){
        VkRenderingAttachmentInfo attachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.resolveMode = VK_RESOLVE_MODE_NONE;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.imageView = *_colorBuffer._[i].imageView;
        attachment.clearValue.color = _clearColor;
        _colorBuffer.attachment[i] = attachment;
    }


    _depthBuffer.attachment = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    _depthBuffer.attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    _depthBuffer.attachment.resolveMode = VK_RESOLVE_MODE_NONE;
    _depthBuffer.attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    _depthBuffer.attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    _depthBuffer.attachment.clearValue.depthStencil = {1.f, 0u};

    _renderingInfo.flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT_KHR;
    _renderingInfo.renderArea = { {0, 0}, { _swapchain->width(), _swapchain->height() }};
    _renderingInfo.layerCount = 1;
    _renderingInfo.colorAttachmentCount = 1;
    _renderingInfo.pDepthAttachment = &_depthBuffer.attachment;

}

void VulkanRenderer::recordScene(VkCommandBuffer sceneCommandBuffer) {
    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    vkBeginCommandBuffer(_renderCommandBuffer[currentFrame], &beginInfo);
    vkCmdBeginRendering(_renderCommandBuffer[currentFrame], &_renderingInfo);

    vkCmdExecuteCommands(_renderCommandBuffer[currentFrame], 1, &sceneCommandBuffer);

    vkCmdEndRendering(_renderCommandBuffer[currentFrame]);
    vkEndCommandBuffer(_renderCommandBuffer[currentFrame]);
}

void VulkanRenderer::clearColor(float r, float g, float b, float a) {
    _clearColor = {r, g, b, a};
    for(auto& attachment : _colorBuffer.attachment){
        attachment.clearValue.color = _clearColor;
    }
}

void VulkanRenderer::stop() {
    destroySynchronizationPrimitives();
}

void VulkanRenderer::invalidateSwapchain() {
    _window->aWaitEvents();

    _device->wait();
    destroySynchronizationPrimitives();

    auto [width, height] = _window->dimensions();
    _swapchainBuilder.setExtent(width, height);
    _swapchain = _swapchainBuilder.make_unique();
    init();
}

uint32 VulkanRenderer::width() const {
    return _swapchain->width();
}

uint32 VulkanRenderer::height() const {
    return _swapchain->height();
}

VkSampleCountFlagBits VulkanRenderer::samples() const {
    return VK_SAMPLE_COUNT_1_BIT; // TODO query renderer / swapchain for this info
}

VkFormat VulkanRenderer::format() const {
    return _swapchain->format();
}

VkFormat VulkanRenderer::depthFormat() const {
    return VK_FORMAT_D32_SFLOAT;
}

uint32 VulkanRenderer::framesInFlight() const {
    return MAX_IN_FLIGHT_FRAMES;
}
