#pragma once

#include "Scene.hpp"
#include "VulkanPipeline.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class CubeScene : public Scene {
public:
    CubeScene(std::shared_ptr<VulkanDevice> device);

    ~CubeScene() override = default;

    void init() final;

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
    std::shared_ptr<VulkanBuffer> _cubeVertices;
    std::shared_ptr<VulkanBuffer> _cubeIndices;
    std::shared_ptr<VulkanBuffer> _transformBuffer;
    std::unique_ptr<VulkanDescriptorPool> _descriptorPool;
    std::unique_ptr<VulkanPipelineDescriptorSetLayout> _descriptorSetLayout;
    std::unique_ptr<VulkanPipelineLayout> _pipelineLayout;
    std::unique_ptr<VulkanPipeline> _pipeline;
    VkDescriptorSet _descriptorSet;

    glm::mat4* transform{};

};