#include "Types.hpp"
#include "Time.hpp"
#include "event/Events.hpp"
#include "Scene.hpp"

#include <utility>

Scene::Scene(std::shared_ptr<VulkanDevice> device, const AppState& appState)
: _device(std::move(device))
, _appState(appState)
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
    static VkFormat format;

    _renderingInfo.colorAttachmentCount = 1;
    _renderingInfo.pColorAttachmentFormats = &format;
    _renderingInfo.depthAttachmentFormat = _appState.screenDepthFormat();
    _renderingInfo.rasterizationSamples = _appState.screenSampleCount();
    _inheritanceInfo.pNext = &_renderingInfo;
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
    beginInfo.pInheritanceInfo = &_inheritanceInfo;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    record(commandBuffer);
    vkEndCommandBuffer(commandBuffer);
}

void Scene::initCommandBuffer() {
    _commandPool->reset();
    _commandBuffer = _commandPool->allocate(_appState.numFramesInFlight(), VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    for(auto cb : _commandBuffer) {
        record0(cb);
    }
}

void Scene::dynamicScene() {
    _dynamicScene = true;
}

std::span<VkCommandBuffer> Scene::record() {
    auto commandBuffer = _commandBuffer[_appState.currentFrame()];
    if(_dynamicScene){
        record0(commandBuffer);
    }
    return { &commandBuffer, 1 };
}

void Scene::record(VkCommandBuffer commandBuffer) {}

void Scene::update() {}

void Scene::invalidate() {}


TestScene::TestScene(std::shared_ptr<VulkanDevice> _device, const AppState& appState)
: Scene(std::move(_device), appState) {}

void TestScene::update() {
    static int period = 5;
    const auto elapsed = Time::Elapsed();

    if(int(elapsed) % period == 0){
        int index = int(elapsed)/period % to<int>(_clearColors.size());
        auto cc = _clearColors[index].float32;
        Events::ClearScreen(cc[0], cc[1], cc[2], cc[3]);
    }
}

