#pragma once

#include "Scene.hpp"
#include "VulkanPipeline.hpp"
#include "Texture.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class CubeScene : public Scene {
public:
    CubeScene(std::shared_ptr<VulkanDevice> device, const AppState& appState);

    ~CubeScene() override = default;

    void init() final;

    void createTexture();

    void update() override;

    void initCamera();

    void createBuffers();

    void createPipeline();

    void createDescriptorPool();

    void createDescriptorSetLayout();

    void updateDescriptorSet();

    void invalidate() override;

    void record(VkCommandBuffer commandBuffer) override;

private:
    std::shared_ptr<VulkanBuffer> cubeVertices_;
    std::shared_ptr<VulkanBuffer> cubeIndices_;
    std::shared_ptr<VulkanBuffer> transformBuffer_;
    std::unique_ptr<VulkanDescriptorPool> descriptorPool_;
    std::unique_ptr<VulkanDescriptorSetLayout> descriptorSetLayout_;
    std::unique_ptr<VulkanPipelineLayout> pipelineLayout_;
    std::unique_ptr<VulkanPipeline> pipeline_;
    VkDescriptorSet descriptorSet_;

    glm::mat4* transform{};
    Texture texture_;

};