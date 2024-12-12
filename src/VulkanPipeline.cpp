#include "VulkanPipeline.hpp"

#include <stdexcept>

VulkanDescriptorPool::VulkanDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool)
        : device_(device)
        , descriptorPool_(descriptorPool)
{}

VulkanDescriptorPool::~VulkanDescriptorPool() {
    vkDestroyDescriptorPool(device_, descriptorPool_, nullptr);
}

VkDescriptorSet VulkanDescriptorPool::allocate(VkDescriptorSetLayout layout) {
    VkDescriptorSet descriptorSet;
    VkDescriptorSetAllocateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    info.descriptorPool = descriptorPool_;
    info.descriptorSetCount = 1;
    info.pSetLayouts = &layout;
    vkAllocateDescriptorSets(device_, &info, &descriptorSet);
    return descriptorSet;
}

std::vector<VkDescriptorSet> VulkanDescriptorPool::allocate(std::span<VkDescriptorSetLayout> layouts) {
    std::vector<VkDescriptorSet> descriptorSet(layouts.size());

    VkDescriptorSetAllocateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    info.descriptorPool = descriptorPool_;
    info.descriptorSetCount = layouts.size();
    info.pSetLayouts = layouts.data();

    vkAllocateDescriptorSets(device_, &info, descriptorSet.data());

    return descriptorSet;
}

VulkanDescriptorPoolCreator::VulkanDescriptorPoolCreator(VkDevice device)
        : device_(device){}

VulkanDescriptorPoolCreator &VulkanDescriptorPoolCreator::flags(VkDescriptorPoolCreateFlags flags) {
    info_.flags = flags;
    return *this;
}

VulkanDescriptorPoolCreator &VulkanDescriptorPoolCreator::maxSets(uint32_t value) {
    info_.maxSets = value;
    return *this;
}

VulkanDescriptorPoolCreator &VulkanDescriptorPoolCreator::addPoolSize(VkDescriptorPoolSize poolSize) {
    poolSizes_.push_back(poolSize);
    return *this;
}

VkDescriptorPool VulkanDescriptorPoolCreator::create() {
    info_.poolSizeCount = poolSizes_.size();
    info_.pPoolSizes = poolSizes_.data();

    VkDescriptorPool descriptorPool;
    auto result = vkCreateDescriptorPool(device_, &info_, nullptr, &descriptorPool);
    if(result != VK_SUCCESS){
        throw std::runtime_error{ "unable to create descriptor pool "};
    }

    return descriptorPool;
}

std::unique_ptr<VulkanDescriptorPool> VulkanDescriptorPoolCreator:: make_unique() {
    return std::make_unique<VulkanDescriptorPool>( device_, create() );
}

std::shared_ptr<VulkanDescriptorPool> VulkanDescriptorPoolCreator::make_shared() {
    return std::make_shared<VulkanDescriptorPool>( device_, create() );
}

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout setLayout)
: device_(device)
, setLayout_(setLayout){}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(device_, setLayout_, VK_NULL_HANDLE);
}

VulkanDescriptorSetLayout::operator VkDescriptorSetLayout() const {
    return setLayout_;
}

VulkanDescriptorSetLayout::operator VkDescriptorSetLayout*() {
    return &setLayout_;
}

VulkanPipelineDescriptorSetLayoutCreator::VulkanPipelineDescriptorSetLayoutCreator(VkDevice device)
: device_(device)
, info_{ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO }{}

VulkanPipelineDescriptorSetLayoutCreator &
VulkanPipelineDescriptorSetLayoutCreator::flags(VkDescriptorSetLayoutCreateFlags flags) {
    info_.flags = flags;
    return *this;
}

VulkanPipelineDescriptorSetLayoutCreator &
VulkanPipelineDescriptorSetLayoutCreator::addBinding(const VkDescriptorSetLayoutBinding &binding) {
    bindings_.push_back(binding);
    return *this;
}

VkDescriptorSetLayout VulkanPipelineDescriptorSetLayoutCreator::create() {
    info_.bindingCount = bindings_.size();
    info_.pBindings = bindings_.data();

    VkDescriptorSetLayout layout{};
    auto result = vkCreateDescriptorSetLayout(device_, &info_, nullptr, &layout);
    if(result != VK_SUCCESS){
        throw std::runtime_error{ "unable to create descriptorset layout" };
    }
    return layout;
}

std::unique_ptr<VulkanDescriptorSetLayout> VulkanPipelineDescriptorSetLayoutCreator::make_unique() {
    VkDescriptorSetLayout setLayout = create();
    return std::make_unique<VulkanDescriptorSetLayout>(device_, setLayout);
}

std::shared_ptr<VulkanDescriptorSetLayout> VulkanPipelineDescriptorSetLayoutCreator::make_shared() {
    VkDescriptorSetLayout setLayout = create();
    return std::make_shared<VulkanDescriptorSetLayout>(device_, setLayout);
}

VulkanPipelineLayout::VulkanPipelineLayout(VkDevice device, VkPipelineLayout layout)
: device_(device)
, layout_(layout){}

VulkanPipelineLayout::~VulkanPipelineLayout() {
    vkDestroyPipelineLayout(device_, layout_, nullptr);
}

VulkanPipelineLayout::operator VkPipelineLayout() const {
    return layout_;
}
VulkanPipelineLayout::operator VkPipelineLayout*() {
    return &layout_;
}

VulkanPipelineLayoutCreator::VulkanPipelineLayoutCreator(VkDevice device)
: device_(device){}

VulkanPipelineLayoutCreator &VulkanPipelineLayoutCreator::addSetLayout(VkDescriptorSetLayout setLayout) {
    setLayouts_.push_back(setLayout);
    return *this;
}

VulkanPipelineLayoutCreator &VulkanPipelineLayoutCreator::addPushConstant(VkPushConstantRange range) {
    ranges_.push_back(range);
    return *this;
}

VkPipelineLayout VulkanPipelineLayoutCreator::create() {
    info_.setLayoutCount = setLayouts_.size();
    info_.pSetLayouts = setLayouts_.data();

    info_.pushConstantRangeCount = ranges_.size();
    info_.pPushConstantRanges = ranges_.data();

    VkPipelineLayout layout;
    auto result = vkCreatePipelineLayout(device_, &info_, nullptr, &layout);

    if(result != VK_SUCCESS){
        throw std::runtime_error{ "unable to create pipeline layout"};
    }

    return layout;
}

VulkanPipelineLayoutCreator &VulkanPipelineLayoutCreator::flags(VkPipelineLayoutCreateFlags flags) {
    info_.flags = flags;
    return *this;
}

std::unique_ptr<VulkanPipelineLayout> VulkanPipelineLayoutCreator::make_unique() {
    return std::make_unique<VulkanPipelineLayout>( device_, create());
}

std::shared_ptr<VulkanPipelineLayout> VulkanPipelineLayoutCreator::make_shared() {
    return std::make_shared<VulkanPipelineLayout>( device_, create());
}

VulkanPipeline::VulkanPipeline(VkDevice device, VkPipeline pipeline)
: device_(device)
, pipeline_(pipeline){}

VulkanPipeline::~VulkanPipeline() {
    vkDestroyPipeline(device_, pipeline_, nullptr);
}

VulkanPipeline::operator VkPipeline() const {
    return pipeline_;
}
