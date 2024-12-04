#pragma once

#include "VulkanShaderModule.hpp"
#include <volk.h>

#include <vector>
#include <memory>
#include <span>

class VulkanDescriptorPool {
public:
    VulkanDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool);

    ~VulkanDescriptorPool();

    VkDescriptorSet allocate(VkDescriptorSetLayout layout);

    std::vector<VkDescriptorSet> allocate(std::span<VkDescriptorSetLayout> layouts);

private:
    VkDevice device_;
    VkDescriptorPool descriptorPool_;
};

class VulkanDescriptorPoolCreator {
public:
    VulkanDescriptorPoolCreator(VkDevice device);

    VulkanDescriptorPoolCreator& flags(VkDescriptorPoolCreateFlags flags);

    VulkanDescriptorPoolCreator& maxSets(uint32_t value);

    VulkanDescriptorPoolCreator& addPoolSize(VkDescriptorPoolSize poolSize);

    VkDescriptorPool create();

    std::unique_ptr<VulkanDescriptorPool> make_unique();

    std::shared_ptr<VulkanDescriptorPool> make_shared();

private:
    VkDevice device_;
    std::vector<VkDescriptorPoolSize> poolSizes_;
    VkDescriptorPoolCreateInfo info_{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
};

class VulkanPipelineDescriptorSetLayout {
public:
    VulkanPipelineDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout setLayout);

    ~VulkanPipelineDescriptorSetLayout();

    operator VkDescriptorSetLayout() const;

    operator VkDescriptorSetLayout*();

private:
    VkDevice device_;
    VkDescriptorSetLayout setLayout_;
};

class VulkanPipelineDescriptorSetLayoutCreator {
public:
    VulkanPipelineDescriptorSetLayoutCreator(VkDevice device);

    VulkanPipelineDescriptorSetLayoutCreator& flags(VkDescriptorSetLayoutCreateFlags flags);

    VulkanPipelineDescriptorSetLayoutCreator& addBinding(const VkDescriptorSetLayoutBinding& binding);

    VkDescriptorSetLayout create();

    std::unique_ptr<VulkanPipelineDescriptorSetLayout> make_unique();

    std::shared_ptr<VulkanPipelineDescriptorSetLayout> make_shared();

private:
    VkDevice device_;
    std::vector<VkDescriptorSetLayoutBinding> bindings_{};
    VkDescriptorSetLayoutCreateInfo info_;
};

class VulkanPipelineLayout {
public:
    explicit VulkanPipelineLayout(VkDevice device, VkPipelineLayout layout_);

    ~VulkanPipelineLayout();

    operator VkPipelineLayout() const;

    operator VkPipelineLayout*();

private:
    VkDevice device_;
    VkPipelineLayout layout_;
};

class VulkanPipelineLayoutCreator {
public:
    VulkanPipelineLayoutCreator(VkDevice device);

    VulkanPipelineLayoutCreator& flags(VkPipelineLayoutCreateFlags  flags);

    VulkanPipelineLayoutCreator& addSetLayout(VkDescriptorSetLayout setLayout);

    VulkanPipelineLayoutCreator& addPushConstant(VkPushConstantRange range);

    VkPipelineLayout create();

    std::unique_ptr<VulkanPipelineLayout> make_unique();

    std::shared_ptr<VulkanPipelineLayout> make_shared();

private:
    VkDevice device_;
    std::vector<VkDescriptorSetLayout> setLayouts_;
    std::vector<VkPushConstantRange> ranges_;
    VkPipelineLayoutCreateInfo info_{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
};

class VulkanPipeline {
public:
    VulkanPipeline(VkDevice device, VkPipeline pipeline);

    ~VulkanPipeline();

    operator VkPipeline() const;

private:
    VkDevice device_;
    VkPipeline pipeline_;
};