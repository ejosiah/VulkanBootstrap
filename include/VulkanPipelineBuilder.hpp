#pragma once

#include "VulkanDevice.hpp"
#include "VulkanPipeline.hpp"

#include <filesystem>
#include <vector>

class VulkanPipelineBuilder {
public:
    VulkanPipelineBuilder(VulkanDeviceSptr device, VulkanDescriptorPoolSptr descriptorPool);

    VulkanPipelineBuilder& adShader(const std::filesystem::path& path);

    VulkanPipelineBuilder& addViewport(float width, float height,float x = 0, float y = 0, float minDepth = 0, float maxDepth = 1);

    VulkanPipelineBuilder& addScissor(VkExtent2D extent, VkOffset2D offset = {0, 0});

private:
    VulkanDeviceSptr device_;
    VulkanDescriptorPoolSptr descriptorPool_;
    std::vector<std::filesystem::path> shaderPaths_;
    std::vector<VkViewport> viewport_;
    VkRect2D scissor_;
};