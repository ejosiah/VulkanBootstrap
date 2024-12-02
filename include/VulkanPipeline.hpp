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
    VkDevice _device;
    VkDescriptorPool _descriptorPool;
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
    VkDevice _device;
    std::vector<VkDescriptorPoolSize> _poolSizes;
    VkDescriptorPoolCreateInfo _info{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
};

class VulkanPipelineDescriptorSetLayout {
public:
    VulkanPipelineDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout setLayout);

    ~VulkanPipelineDescriptorSetLayout();

    operator VkDescriptorSetLayout() const;

    operator VkDescriptorSetLayout*();

private:
    VkDevice _device;
    VkDescriptorSetLayout _setLayout;
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
    VkDevice _device;
    std::vector<VkDescriptorSetLayoutBinding> _bindings{};
    VkDescriptorSetLayoutCreateInfo _info;
};

class VulkanPipelineLayout {
public:
    explicit VulkanPipelineLayout(VkDevice device, VkPipelineLayout _layout);

    operator VkPipelineLayout() const;

    operator VkPipelineLayout*();

private:
    VkDevice _device;
    VkPipelineLayout _layout;
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
    VkDevice _device;
    std::vector<VkDescriptorSetLayout> _setLayouts;
    std::vector<VkPushConstantRange> _ranges;
    VkPipelineLayoutCreateInfo _info{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
};

class VulkanPipeline {
public:
    VulkanPipeline(VkDevice device, VkPipeline pipeline);

    ~VulkanPipeline();

    operator VkPipeline() const;

private:
    VkDevice _device;
    VkPipeline _pipeline;
};