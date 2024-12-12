#pragma once

#include "VulkanDevice.hpp"
#include "VulkanPipeline.hpp"

class VulkanPipelineBuilder {
public:
    VulkanPipelineBuilder(VulkanDeviceSptr device, VulkanDescriptorPoolSptr descriptorPool);

    VulkanPipelineBuilder& adShader(const std::filesystem::path& path);



private:
    VulkanDeviceSptr device_;
    VulkanDescriptorPoolSptr descriptorPool_;
    std::vector<std::filesystem::path> shaderPaths_;
};