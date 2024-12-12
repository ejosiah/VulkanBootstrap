#include "Types.hpp"
#include "VulkanDevice.hpp"
#include "VulkanEnumerations.hpp"
#include "Texture.hpp"
#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"
#include "util/Bits.hpp"
#include "SetVulkanObjectName.hpp"
#include <utility>
#include <format>
#include <ranges>

VulkanDevice::VulkanDevice(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device,
                           VmaAllocator allocator, std::map<VkQueueFlags, uint32> queueFamilyIndex,
                           std::map<VkQueueFlags, VkQueue> queues, std::unique_ptr<VulkanCommandPool> graphicsCommandPool)

:instance_(instance)
, physicalDevice_(physicalDevice)
, device_(device)
, allocator_(allocator)
, queueFamilyIndex_(std::move(queueFamilyIndex))
, queues_(std::move(queues))
, graphicsCommandPool_(std::move(graphicsCommandPool))
{}

VulkanDevice::~VulkanDevice() {
    graphicsCommandPool_.reset();
    vmaDestroyAllocator(allocator_);
    vkDestroyDevice(device_, VK_NULL_HANDLE);
}

std::string VulkanDevice::toString(VkPhysicalDeviceType deviceType) {
    switch(deviceType){
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "Integrated GPU";
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: return "Discrete GPU";
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: return "Virtual GPU";
        case VK_PHYSICAL_DEVICE_TYPE_CPU: return "CPU";
        default: return "Unknown";
    }
}

std::string VulkanDevice::toString() {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevice_, &props);

    auto apiVersion = std::format("{}.{}.{}",
                                  VK_API_VERSION_MAJOR(props.apiVersion),
                                  VK_API_VERSION_MINOR(props.apiVersion),
                                  VK_API_VERSION_PATCH(props.apiVersion));

    auto driverVersion = std::format("{}.{}.{}",
                                     VK_API_VERSION_MAJOR(props.driverVersion),
                                     VK_API_VERSION_MINOR(props.driverVersion),
                                     VK_API_VERSION_PATCH(props.driverVersion));


    return std::format(
            "device Info:\n\tname:{}\n\tAPI version:{}\n\tDriver version: {}\n\tType: {}"
            , props.deviceName, apiVersion, driverVersion, VulkanDevice::toString(props.deviceType));
}

VulkanDeviceBuilder VulkanDevice::builder(VkInstance instance) {
    return { instance };
}

VkQueue VulkanDevice::getQueue(uint32 queueFamilyIndex) const {
    auto res = std::ranges::find_if(queueFamilyIndex_, [i=queueFamilyIndex](auto e){ return e.second == i; });
    if(res != queueFamilyIndex_.end()){
        return queues_.at(res->first);
    }else {
        return nullptr;
    }
}

VkQueue VulkanDevice::getGraphicsQueue() const {
    if(!queues_.contains(VK_QUEUE_GRAPHICS_BIT)) {
        return nullptr;
    }
    return queues_.at(VK_QUEUE_GRAPHICS_BIT);
}

VkQueue VulkanDevice::getComputeQueue() const {
    if(!queues_.contains(VK_QUEUE_COMPUTE_BIT)) {
        return nullptr;
    }
    return queues_.at(VK_QUEUE_COMPUTE_BIT);
}

VkQueue VulkanDevice::getTransferQueue() const {
    if(!queues_.contains(VK_QUEUE_TRANSFER_BIT)) {
        return nullptr;
    }
    return queues_.at(VK_QUEUE_TRANSFER_BIT);
}

VkQueue VulkanDevice::getPresentQueue() const {
    if(!queues_.contains(VK_QUEUE_PRESENT_BIT)) {
        return nullptr;
    }
    return queues_.at(VK_QUEUE_PRESENT_BIT);
}

VkSurfaceCapabilitiesKHR VulkanDevice::getSurfaceCapabilities(VkSurfaceKHR surface) const {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_, surface, &capabilities);
    return capabilities;
}

std::vector<VkSurfaceFormatKHR> VulkanDevice::getSurfaceFormat(VkSurfaceKHR surface) const {
    uint32 count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface, &count, nullptr);

    std::vector<VkSurfaceFormatKHR> formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface, &count, formats.data());
    return formats;
}

std::vector<VkPresentModeKHR> VulkanDevice::getSurfacePresentationsModes(VkSurfaceKHR surface) const {
    uint32 count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_, surface, &count, nullptr);

    std::vector<VkPresentModeKHR> modes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_, surface, &count, modes.data());
    return modes;
}

VkDevice VulkanDevice::handle() const {
    return device_;
}

std::shared_ptr<VulkanCommandPool>
VulkanDevice::createCommandPool(VkQueueFlagBits queueFlag, VkCommandPoolCreateFlags flags) {
    if(!queues_.contains(queueFlag)){
        throw std::runtime_error{ std::format("No queue defined for queue type, TODO queue flag to string") };
    }

    return VulkanCommandPool::make_shared(device_, queues_.at(queueFlag), queueFamilyIndex_.at(queueFlag), flags);
}

VulkanImageCreator VulkanDevice::image() {
    return {device_, allocator_ };
}

VulkanImageViewCreator VulkanDevice::imageView() {
    return VulkanImageViewCreator{ device_ };
}

VulkanBufferCreator VulkanDevice::buffer() {
    return { device_, allocator_ };
}

VulkanDevice::operator VkDevice() const {
    return device_;
}

void VulkanDevice::wait() {
    vkDeviceWaitIdle(device_);
}

VulkanShaderModuleCreator VulkanDevice::shader() {
    return { device_ };
}

VulkanDescriptorPoolCreator VulkanDevice::descriptorPool() {
    return { device_ };
}

VulkanPipelineDescriptorSetLayoutCreator VulkanDevice::descriptorSetLayout() {
    return { device_ };
}

VulkanPipelineLayoutCreator VulkanDevice::pipelineLayout() {
    return { device_ };
}

std::unique_ptr<VulkanPipeline> VulkanDevice::graphicsPipeline(VkGraphicsPipelineCreateInfo createInfo) {
    VkPipeline pipeline;
    auto result =  vkCreateGraphicsPipelines(device_, nullptr, 1, &createInfo, nullptr, &pipeline);
    if(result != VK_SUCCESS) {
        throw std::runtime_error{ "unable to create graphics pipeline" };
    }
    return std::make_unique<VulkanPipeline>(device_, pipeline);
}

VkPhysicalDeviceProperties VulkanDevice::getProperties() const {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice_, &properties);
    return properties;
}

VulkanCommandPool* VulkanDevice::graphicsCommandPool() {
    return graphicsCommandPool_.get();
}

TextureBuilder VulkanDevice::texture() {
    return TextureBuilder(this);
}

VulkanDeviceBuilder::VulkanDeviceBuilder(VkInstance instance)
: instance_(instance){}

VulkanDeviceBuilder& VulkanDeviceBuilder::addSurface(VkSurfaceKHR surface) {
    surface_ = surface;
    return *this;
}

VulkanDeviceBuilder &VulkanDeviceBuilder::addQueue(VkQueueFlagBits queueType) {
     queueTypes_ |= queueType;
    return *this;
}

VulkanDeviceBuilder &VulkanDeviceBuilder::addUniqueQueue(VkQueueFlagBits queueType) {
    uniqueQueueTypes_ |= queueType;

    return *this;
}


VulkanDeviceBuilder &VulkanDeviceBuilder::addLayer(const char *layer) {
    enabledLayers_.push_back(layer);
    return *this;
}

VulkanDeviceBuilder &VulkanDeviceBuilder::addExtension(const char *extension) {
    enabledExtensions_.push_back(extension);
    return *this;
}

VulkanDeviceBuilder &VulkanDeviceBuilder::addLayers(std::vector<const char *> layers) {
    enabledLayers_.insert(enabledLayers_.end(), layers.begin(), layers.end());
    return *this;
}

VulkanDeviceBuilder &VulkanDeviceBuilder::addExtensions(std::vector<const char *> extensions) {
    enabledExtensions_.insert(enabledExtensions_.end(), extensions.begin(), extensions.end());
    return *this;
}

VkPhysicalDevice VulkanDeviceBuilder::pickDevice(VulkanDeviceBuilder::DevicePicker &&pick) {
    auto physicalDevices = v_enumerate<VkPhysicalDevice>(vkEnumeratePhysicalDevices, instance_);
    physicalDevice_ = pick(physicalDevices);
    return physicalDevice_;
}

std::shared_ptr<VulkanDevice> VulkanDeviceBuilder::make_shared() {
    if(!physicalDevice_) {
        pickDevice();
    }
    auto queueFamilyIndex = getQueueFamilyIndexes();
    auto device = createDevice(queueFamilyIndex);
    auto allocator = createAllocator(device);
    std::map<VkQueueFlags, VkQueue> queues;
    for(auto [flag, index] : queueFamilyIndex){
        VkQueue queue;
        vkGetDeviceQueue(device, index, 0, &queue);
        queues[flag] = queue;
    }
    auto graphicsCommandPool = VulkanCommandPool::make_unique(device, queues.at(VK_QUEUE_GRAPHICS_BIT)
                                                                                    , queueFamilyIndex.at(VK_QUEUE_GRAPHICS_BIT),
                                                                                            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    setVulkanObjectName(device, VK_OBJECT_TYPE_COMMAND_POOL, graphicsCommandPool->handle(), "device_graphics_command_pool");

    return std::make_shared<VulkanDevice>(instance_, physicalDevice_, device, allocator, queueFamilyIndex, queues, std::move(graphicsCommandPool));
}

std::map<VkQueueFlags, uint32> VulkanDeviceBuilder::getQueueFamilyIndexes() {
    assert(queueTypes_ != 0 && "queueTypes not set");
    const auto props = v_enumerate<VkQueueFamilyProperties>(vkGetPhysicalDeviceQueueFamilyProperties, physicalDevice_);

    std::map<VkFlags, uint32> queueFamilyIndex;

    auto queryQueue = [&, queueType = queueTypes_](auto queueFamily, auto queueFlagBits, auto qfIndex){
        if(!queueFamilyIndex.contains(queueFlagBits)){
            if((queueType & queueFlagBits) == queueFlagBits && (queueFamily.queueFlags & queueFlagBits) == queueFlagBits ){
                queueFamilyIndex[queueFlagBits] = qfIndex;
            }
        }
    };

    for(auto i = 0u; i < props.size(); ++i){
        auto queueFamily = props[i];
        queryQueue(queueFamily, VK_QUEUE_GRAPHICS_BIT, i);
        queryQueue(queueFamily, VK_QUEUE_COMPUTE_BIT, i);
        queryQueue(queueFamily, VK_QUEUE_TRANSFER_BIT, i);

        if(surface_ && !queueFamilyIndex.contains(VK_QUEUE_PRESENT_BIT)){
            VkBool32 presentSupported;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice_, i, surface_, &presentSupported);
            if(presentSupported){
                queueFamilyIndex[VK_QUEUE_PRESENT_BIT] = i;
            }
        }
    }



    auto queryUniqueQueue =
            [&, queueType = uniqueQueueTypes_, previousQueueFamily = std::map<VkFlags, uint32>{}]
            (auto queueFamily, auto queueFlagBits, auto qfIndex) mutable  {

                auto count = bits::count(queueFamily.queueFlags);
                if((queueType & queueFlagBits) == queueFlagBits && (queueFamily.queueFlags & queueFlagBits) == queueFlagBits) {
                    if(!previousQueueFamily.contains(queueFlagBits)){
                        previousQueueFamily[queueFlagBits] = count;
                        queueFamilyIndex[queueFlagBits] = qfIndex;
                    }else {
                        auto previousCount = previousQueueFamily[queueFlagBits];
                        if(count < previousCount){
                            previousQueueFamily[queueFlagBits] = count;
                            queueFamilyIndex[queueFlagBits] = qfIndex;
                        }
                    }
                }
            };


    for(auto i = 0u; i < props.size(); ++i) {
        queryUniqueQueue(props[i], VK_QUEUE_COMPUTE_BIT, i);
        queryUniqueQueue(props[i], VK_QUEUE_TRANSFER_BIT, i);
    }

    return queueFamilyIndex;
}

VkDevice VulkanDeviceBuilder::createDevice(const std::map<VkQueueFlags, uint32>& queueFamilyIndexes) {
    VkPhysicalDeviceVulkan13Features devFeatures13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    devFeatures13.synchronization2 = VK_TRUE;
    devFeatures13.dynamicRendering = VK_TRUE;
    devFeatures13.maintenance4 = VK_TRUE;


    auto queuePriority = 1.0f;
    std::set<uint32> uniqueQueueFamilyIndexes;
    for(auto [_, queueFamilyIndex] : queueFamilyIndexes) {
        uniqueQueueFamilyIndexes.insert(queueFamilyIndex);
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    for(auto queueFamilyIndex: uniqueQueueFamilyIndexes) {
        VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
        queueInfo.queueFamilyIndex = queueFamilyIndex;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueInfo);
    }

    VkDeviceCreateInfo createInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    createInfo.pNext = &devFeatures13;
    createInfo.queueCreateInfoCount = to<uint32>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.enabledLayerCount = to<uint32>(enabledLayers_.size());
    createInfo.ppEnabledLayerNames = enabledLayers_.data();
    createInfo.enabledExtensionCount = to<uint32>(enabledExtensions_.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions_.data();
    createInfo.pEnabledFeatures = &enabledFeatures_;

    VkDevice device;
    auto result = vkCreateDevice(physicalDevice_, &createInfo, VK_NULL_HANDLE, &device);

    if(result != VK_SUCCESS){
        return nullptr;
    }

    return device;
}

VmaAllocator VulkanDeviceBuilder::createAllocator(VkDevice device) {

    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    allocatorCreateInfo.physicalDevice = physicalDevice_;
    allocatorCreateInfo.device = device;
    allocatorCreateInfo.instance = instance_;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    VmaAllocator allocator;
    vmaCreateAllocator(&allocatorCreateInfo, &allocator);
    return allocator;
}