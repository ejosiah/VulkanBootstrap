#include "Types.hpp"
#include "Time.hpp"
#include "event/Events.hpp"
#include "Scene.hpp"

#include <utility>

Scene::Scene(std::shared_ptr<VulkanDevice> device, const AppState& appState, BatchSubmission& batchSubmission)
: device_(std::move(device))
, appState_(appState)
, batchSubmission_(batchSubmission)
, inheritanceInfo_{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
    .pNext = &renderingInfo_
}{}

void Scene::init0() {
    commandPool_ = device_->createCommandPool(VK_QUEUE_GRAPHICS_BIT);
    invalidate0();
    init();
    initCommandBuffer();
}

void Scene::invalidate0() {
    renderingInfo_.colorAttachmentCount = 1;
    renderingInfo_.pColorAttachmentFormats = &appState_.screenFormat();
    renderingInfo_.depthAttachmentFormat = appState_.screenDepthFormat();
    renderingInfo_.rasterizationSamples = appState_.screenSampleCount();
    inheritanceInfo_.pNext = &renderingInfo_;
}

void Scene::refresh() {
    invalidate0();
    invalidate();
    initCommandBuffer();
}

void Scene::record0(VkCommandBuffer commandBuffer) {
    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
    };
    beginInfo.pInheritanceInfo = &inheritanceInfo_;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    record(commandBuffer);
    vkEndCommandBuffer(commandBuffer);
}

void Scene::initCommandBuffer() {
    commandPool_->reset();
    commandBuffer_ = commandPool_->allocate(appState_.numFramesInFlight(), VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    for(auto cb : commandBuffer_) {
        record0(cb);
    }
}

void Scene::dynamicScene() {
    dynamicScene_ = true;
}

std::span<VkCommandBuffer> Scene::record() {
    auto commandBuffer = commandBuffer_[appState_.currentFrame()];
    if(dynamicScene_){
        record0(commandBuffer);
    }
    return { &commandBuffer, 1 };
}

void Scene::record(VkCommandBuffer commandBuffer) {}

void Scene::update() {}

void Scene::invalidate() {}

void Scene::batch(VkCommandBuffer commandBuffer) {
    batchSubmission_.enqueue(commandBuffer);
}

void Scene::addWait(VkSemaphore semaphore, VkPipelineStageFlags flags) {
    batchSubmission_.enqueueWait(semaphore, flags);
}

void Scene::signal(VkSemaphore semaphore) {
    batchSubmission_.enqueueSignal(semaphore);
}

void Scene::init() {}


TestScene::TestScene(std::shared_ptr<VulkanDevice> device_, const AppState& appState, BatchSubmission& batchSubmission)
: Scene(std::move(device_), appState, batchSubmission) {}

void TestScene::update() {
    static int period = 5;
    const auto elapsed = Time::instance().elapsed().count() /1000.f;

    if(int(elapsed) % period == 0){
        int index = int(elapsed)/period % to<int>(clearColors_.size());
        auto cc = clearColors_[index].float32;
        Events::ClearScreen(cc[0], cc[1], cc[2], cc[3]);
    }
}

