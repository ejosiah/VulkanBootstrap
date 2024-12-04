#pragma once

#include "Types.hpp"
#include "event/Events.hpp"
#include "VulkanDevice.hpp"
#include "AppState.hpp"

#include <span>

class Scene {
public:
    Scene(std::shared_ptr<VulkanDevice> device, const AppState& appState);

    virtual ~Scene() = default;

    void init0();

    void initCommandBuffer();

    virtual void init() = 0;

    virtual std::span<VkCommandBuffer> record();

    void record0(VkCommandBuffer commandBuffer);

    virtual void record(VkCommandBuffer commandBuffer);

    virtual void update();

    void invalidate0();

    void refresh();

    virtual void invalidate();

protected:
    void dynamicScene();

protected:
    std::shared_ptr<VulkanDevice> device_;
    const AppState& appState_;
    std::shared_ptr<VulkanCommandPool> commandPool_;
    VkCommandBufferInheritanceInfo inheritanceInfo_;
    VkCommandBufferInheritanceRenderingInfo renderingInfo_{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO };
    bool dynamicScene_{};

private:
    std::vector<VkCommandBuffer> commandBuffer_;
};

class TestScene : public Scene {
public:
    TestScene(std::shared_ptr<VulkanDevice> device_, const AppState& appState);

    void update() final;

private:
    std::vector<VkClearColorValue> clearColors_{ {1.f, 0, 0, 1.f}, {0, 1.f, 0, 1.f}, {0, 0, 1.f, 1.f} };

};