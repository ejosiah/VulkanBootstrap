#include "Types.hpp"
#include "Time.hpp"
#include "event/Events.hpp"
#include "Scene.hpp"

#include <utility>

Scene::Scene(std::shared_ptr<VulkanDevice> device)
: _device(std::move(device))
, _inheritanceInfo{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
    .pNext = &_renderingInfo
}{}

void Scene::init0() {
    _commandPool = _device->createCommandPool(VK_QUEUE_GRAPHICS_BIT);
    invalidate0();
    init();
    initCommandBuffer();
}

void Scene::invalidate0() {
    _renderingInfo.colorAttachmentCount = 1;
    _renderingInfo.pColorAttachmentFormats = &AppState::screenFormat;
    _renderingInfo.depthAttachmentFormat = AppState::screenDepthFormat;
    _renderingInfo.rasterizationSamples = AppState::screenSampleCount;
    _inheritanceInfo.pNext = &_renderingInfo;
}

void Scene::refresh() {
    invalidate0();
    invalidate();
}

void Scene::record0(VkCommandBuffer commandBuffer) {
    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
    };
    beginInfo.pInheritanceInfo = &_inheritanceInfo;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    record(commandBuffer);
    vkEndCommandBuffer(commandBuffer);
}

void Scene::initCommandBuffer() {
    _commandBuffer = _commandPool->allocate(AppState::numFramesInFlight, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    for(auto cb : _commandBuffer) {
        record0(cb);
    }
}

void Scene::dynamicScene() {
    _dynamicScene = true;
}

std::span<VkCommandBuffer> Scene::record() {
    auto commandBuffer = _commandBuffer[AppState::currentFrame];
    if(_dynamicScene){
        record0(commandBuffer);
    }
    return { &commandBuffer, 1 };
}


TestScene::TestScene(std::shared_ptr<VulkanDevice> _device)
: Scene(std::move(_device)) {}

void TestScene::update() {
    static int period = 5;
    const auto elapsed = Time::Elapsed();

    if(int(elapsed) % period == 0){
        int index = int(elapsed)/period % to<int>(_clearColors.size());
        auto cc = _clearColors[index].float32;
        Events::ClearScreen(cc[0], cc[1], cc[2], cc[3]);
    }
}

