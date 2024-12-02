#pragma once

#include "Types.hpp"
#include "event/Events.hpp"
#include "VulkanDevice.hpp"
#include "AppState.hpp"

#include <span>

class Scene {
public:
    Scene(std::shared_ptr<VulkanDevice> _device);

    virtual ~Scene() = default;

    void init0();

    void initCommandBuffer();

    virtual void init() {}

    virtual std::span<VkCommandBuffer> record() {
        auto commandBuffer = _commandBuffer[AppState::currentFrame];
        if(_dynamicScene){
            record0(commandBuffer);
        }
        return { &commandBuffer, 1 };
    };

    void record0(VkCommandBuffer commandBuffer);

    virtual void record(VkCommandBuffer commandBuffer) {};

    virtual void update() {};

    void invalidate0();

    void refresh();

    virtual void invalidate(){};

protected:
    void dynamicScene();

protected:
    std::shared_ptr<VulkanDevice> _device;
    std::shared_ptr<VulkanCommandPool> _commandPool;
    VkCommandBufferInheritanceInfo _inheritanceInfo;
    VkCommandBufferInheritanceRenderingInfo _renderingInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO };
    bool _dynamicScene{};

private:
    std::vector<VkCommandBuffer> _commandBuffer;
};

class TestScene : public Scene {
public:
    TestScene(std::shared_ptr<VulkanDevice> _device);

    void update() final;

private:
    std::vector<VkClearColorValue> _clearColors{ {1.f, 0, 0, 1.f}, {0, 1.f, 0, 1.f}, {0, 0, 1.f, 1.f} };

};