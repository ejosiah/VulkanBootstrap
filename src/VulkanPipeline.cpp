#include "VulkanPipeline.hpp"

#include <stdexcept>

VulkanDescriptorPool::VulkanDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool)
        : _device(device)
        , _descriptorPool(descriptorPool)
{}

VulkanDescriptorPool::~VulkanDescriptorPool() {
    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
}

VkDescriptorSet VulkanDescriptorPool::allocate(VkDescriptorSetLayout layout) {
    VkDescriptorSet descriptorSet;
    VkDescriptorSetAllocateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    info.descriptorPool = _descriptorPool;
    info.descriptorSetCount = 1;
    info.pSetLayouts = &layout;
    vkAllocateDescriptorSets(_device, &info, &descriptorSet);
    return descriptorSet;
}

std::vector<VkDescriptorSet> VulkanDescriptorPool::allocate(std::span<VkDescriptorSetLayout> layouts) {
    std::vector<VkDescriptorSet> descriptorSet(layouts.size());

    VkDescriptorSetAllocateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    info.descriptorPool = _descriptorPool;
    info.descriptorSetCount = layouts.size();
    info.pSetLayouts = layouts.data();

    vkAllocateDescriptorSets(_device, &info, descriptorSet.data());

    return descriptorSet;
}

VulkanDescriptorPoolCreator::VulkanDescriptorPoolCreator(VkDevice device)
        : _device(device){}

VulkanDescriptorPoolCreator &VulkanDescriptorPoolCreator::flags(VkDescriptorPoolCreateFlags flags) {
    _info.flags = flags;
    return *this;
}

VulkanDescriptorPoolCreator &VulkanDescriptorPoolCreator::maxSets(uint32_t value) {
    _info.maxSets = value;
    return *this;
}

VulkanDescriptorPoolCreator &VulkanDescriptorPoolCreator::addPoolSize(VkDescriptorPoolSize poolSize) {
    _poolSizes.push_back(poolSize);
    return *this;
}

VkDescriptorPool VulkanDescriptorPoolCreator::create() {
    _info.poolSizeCount = _poolSizes.size();
    _info.pPoolSizes = _poolSizes.data();

    VkDescriptorPool descriptorPool;
    auto result = vkCreateDescriptorPool(_device, &_info, nullptr, &descriptorPool);
    if(result != VK_SUCCESS){
        throw std::runtime_error{ "unable to create descriptor pool "};
    }

    return descriptorPool;
}

std::unique_ptr<VulkanDescriptorPool> VulkanDescriptorPoolCreator::make_unique() {
    return std::make_unique<VulkanDescriptorPool>( _device, create() );
}

std::shared_ptr<VulkanDescriptorPool> VulkanDescriptorPoolCreator::make_shared() {
    return std::make_shared<VulkanDescriptorPool>( _device, create() );
}

VulkanPipelineDescriptorSetLayout::VulkanPipelineDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout setLayout)
: _device(device)
, _setLayout(setLayout){}

VulkanPipelineDescriptorSetLayout::~VulkanPipelineDescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(_device, _setLayout, VK_NULL_HANDLE);
}

VulkanPipelineDescriptorSetLayout::operator VkDescriptorSetLayout() const {
    return _setLayout;
}

VulkanPipelineDescriptorSetLayout::operator VkDescriptorSetLayout*() {
    return &_setLayout;
}

VulkanPipelineDescriptorSetLayoutCreator::VulkanPipelineDescriptorSetLayoutCreator(VkDevice device)
: _device(device)
, _info{ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO }{}

VulkanPipelineDescriptorSetLayoutCreator &
VulkanPipelineDescriptorSetLayoutCreator::flags(VkDescriptorSetLayoutCreateFlags flags) {
    _info.flags = flags;
    return *this;
}

VulkanPipelineDescriptorSetLayoutCreator &
VulkanPipelineDescriptorSetLayoutCreator::addBinding(const VkDescriptorSetLayoutBinding &binding) {
    _bindings.push_back(binding);
    return *this;
}

VkDescriptorSetLayout VulkanPipelineDescriptorSetLayoutCreator::create() {
    _info.bindingCount = _bindings.size();
    _info.pBindings = _bindings.data();

    VkDescriptorSetLayout layout{};
    auto result = vkCreateDescriptorSetLayout(_device, &_info, nullptr, &layout);
    if(result != VK_SUCCESS){
        throw std::runtime_error{ "unable to create descriptorset layout" };
    }
    return layout;
}

std::unique_ptr<VulkanPipelineDescriptorSetLayout> VulkanPipelineDescriptorSetLayoutCreator::make_unique() {
    VkDescriptorSetLayout setLayout = create();
    return std::make_unique<VulkanPipelineDescriptorSetLayout>(_device, setLayout);
}

std::shared_ptr<VulkanPipelineDescriptorSetLayout> VulkanPipelineDescriptorSetLayoutCreator::make_shared() {
    VkDescriptorSetLayout setLayout = create();
    return std::make_shared<VulkanPipelineDescriptorSetLayout>(_device, setLayout);
}

VulkanPipelineLayout::VulkanPipelineLayout(VkDevice device, VkPipelineLayout layout)
: _device(device)
, _layout(layout){}

VulkanPipelineLayout::operator VkPipelineLayout() const {
    return _layout;
}
VulkanPipelineLayout::operator VkPipelineLayout*() {
    return &_layout;
}

VulkanPipelineLayoutCreator::VulkanPipelineLayoutCreator(VkDevice device)
: _device(device){}

VulkanPipelineLayoutCreator &VulkanPipelineLayoutCreator::addSetLayout(VkDescriptorSetLayout setLayout) {
    _setLayouts.push_back(setLayout);
    return *this;
}

VulkanPipelineLayoutCreator &VulkanPipelineLayoutCreator::addPushConstant(VkPushConstantRange range) {
    _ranges.push_back(range);
    return *this;
}

VkPipelineLayout VulkanPipelineLayoutCreator::create() {
    _info.setLayoutCount = _setLayouts.size();
    _info.pSetLayouts = _setLayouts.data();

    _info.pushConstantRangeCount = _ranges.size();
    _info.pPushConstantRanges = _ranges.data();

    VkPipelineLayout layout;
    auto result = vkCreatePipelineLayout(_device, &_info, nullptr, &layout);

    if(result != VK_SUCCESS){
        throw std::runtime_error{ "unable to create pipeline layout"};
    }

    return layout;
}

VulkanPipelineLayoutCreator &VulkanPipelineLayoutCreator::flags(VkPipelineLayoutCreateFlags flags) {
    _info.flags = flags;
    return *this;
}

std::unique_ptr<VulkanPipelineLayout> VulkanPipelineLayoutCreator::make_unique() {
    return std::make_unique<VulkanPipelineLayout>( _device, create());
}

std::shared_ptr<VulkanPipelineLayout> VulkanPipelineLayoutCreator::make_shared() {
    return std::make_shared<VulkanPipelineLayout>( _device, create());
}

VulkanPipeline::VulkanPipeline(VkDevice device, VkPipeline pipeline)
: _device(device)
, _pipeline(pipeline){}

VulkanPipeline::~VulkanPipeline() {
    vkDestroyPipeline(_device, _pipeline, nullptr);
}

VulkanPipeline::operator VkPipeline() const {
    return _pipeline;
}
