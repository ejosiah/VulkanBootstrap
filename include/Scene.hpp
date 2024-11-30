#pragma once

#include "Types.hpp"
#include <VulkanDevice.hpp>

#include <span>

class Scene {
public:
    Scene(std::shared_ptr<VulkanDevice> _device);

    virtual ~Scene() = default;

    void init0();

    virtual void init() {}

    virtual std::span<VkCommandBuffer> record() {
        return {};
    };

    virtual void update() {};

protected:
    std::shared_ptr<VulkanDevice> _device;
    std::shared_ptr<VulkanCommandPool> _commandPool;
    uint32 width;
    uint32 height;
};

class TestScene : public Scene {
public:
    TestScene(std::shared_ptr<VulkanDevice> _device);

    void update() final;

private:
    std::vector<VkClearColorValue> _clearColors{ {1.f, 0, 0, 1.f}, {0, 1.f, 0, 1.f}, {0, 0, 1.f, 1.f} };
    VkClearColorValue _clearColor;

};