#pragma once

#include "Types.hpp"
#include "event/Events.hpp"
#include "VulkanDevice.hpp"
#include "Properties.hpp"

#include <span>

class Scene {
public:
    Scene(std::shared_ptr<VulkanDevice> _device);

    virtual ~Scene() = default;

    void init0();

    virtual void init() {}

    virtual std::span<VkCommandBuffer> record() {
        VkCommandBufferBeginInfo info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        vkBeginCommandBuffer(_commandBuffer, &info);
        vkEndCommandBuffer(_commandBuffer);
        return { &_commandBuffer, 1 };
    };

    virtual void update() {};

    void set(std::shared_ptr<Properties> properties);

    void invalidate0();

    void refresh();

    virtual void invalidate(){};

protected:
    std::shared_ptr<VulkanDevice> _device;
    std::shared_ptr<VulkanCommandPool> _commandPool;
    std::shared_ptr<Properties> _properties;
    VkCommandBufferInheritanceInfo _inheritanceInfo;
    VkCommandBufferInheritanceRenderingInfo _renderingInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO };
    uint32 width;
    uint32 height;

private:
    VkCommandBuffer _commandBuffer{};
};

class TestScene : public Scene {
public:
    TestScene(std::shared_ptr<VulkanDevice> _device);

    void update() final;

private:
    std::vector<VkClearColorValue> _clearColors{ {1.f, 0, 0, 1.f}, {0, 1.f, 0, 1.f}, {0, 0, 1.f, 1.f} };

};