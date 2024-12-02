#include "Types.hpp"
#include "VulkanDevice.hpp"
#include "VulkanEnumerations.hpp"
#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"
#include <utility>
#include <format>

VulkanDevice::VulkanDevice(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device,
                           VmaAllocator allocator, std::map<VkQueueFlags, uint32> queueFamilyIndex,
                           std::map<VkQueueFlags, VkQueue> queues)

:_instance(instance)
, _physicalDevice(physicalDevice)
, _device(device)
, _allocator(allocator)
, _queueFamilyIndex(std::move(queueFamilyIndex))
, _queues(std::move(queues))
{}

VulkanDevice::~VulkanDevice() {
    vmaDestroyAllocator(_allocator);
    vkDestroyDevice(_device, VK_NULL_HANDLE);
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
    vkGetPhysicalDeviceProperties(_physicalDevice, &props);

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
    auto itr = std::find_if(_queueFamilyIndex.begin(), _queueFamilyIndex.end(),
                            [i=queueFamilyIndex](auto e){ return e.second == i; });
    if(itr != _queueFamilyIndex.end()){
        return _queues.at(itr->first);
    }else {
        return nullptr;
    }
}

VkQueue VulkanDevice::getGraphicsQueue() const {
    if(!_queues.contains(VK_QUEUE_GRAPHICS_BIT)) {
        return nullptr;
    }
    return _queues.at(VK_QUEUE_GRAPHICS_BIT);
}

VkQueue VulkanDevice::getComputeQueue() const {
    if(!_queues.contains(VK_QUEUE_COMPUTE_BIT)) {
        return nullptr;
    }
    return _queues.at(VK_QUEUE_COMPUTE_BIT);
}

VkQueue VulkanDevice::getTransferQueue() const {
    if(!_queues.contains(VK_QUEUE_TRANSFER_BIT)) {
        return nullptr;
    }
    return _queues.at(VK_QUEUE_TRANSFER_BIT);
}

VkQueue VulkanDevice::getPresentQueue() const {
    if(!_queues.contains(VK_QUEUE_PRESENT_BIT)) {
        return nullptr;
    }
    return _queues.at(VK_QUEUE_PRESENT_BIT);
}

VkSurfaceCapabilitiesKHR VulkanDevice::getSurfaceCapabilities(VkSurfaceKHR surface) const {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, surface, &capabilities);
    return capabilities;
}

std::vector<VkSurfaceFormatKHR> VulkanDevice::getSurfaceFormat(VkSurfaceKHR surface) const {
    uint32 count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, surface, &count, nullptr);

    std::vector<VkSurfaceFormatKHR> formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, surface, &count, formats.data());
    return formats;
}

std::vector<VkPresentModeKHR> VulkanDevice::getSurfacePresentationsModes(VkSurfaceKHR surface) const {
    uint32 count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, surface, &count, nullptr);

    std::vector<VkPresentModeKHR> modes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, surface, &count, modes.data());
    return modes;
}

VkDevice VulkanDevice::handle() const {
    return _device;
}

std::shared_ptr<VulkanCommandPool>
VulkanDevice::createCommandPool(VkQueueFlagBits queueFlag, VkCommandPoolCreateFlags flags) {
    if(!_queues.contains(queueFlag)){
        throw std::runtime_error{ std::format("No queue defined for queue type, TODO queue flag to string") };
    }

    return VulkanCommandPool::make_shared(_device, _queues.at(queueFlag), _queueFamilyIndex.at(queueFlag), flags);
}

VulkanImageCreator VulkanDevice::image() {
    return {_device, _allocator };
}

VulkanImageViewCreator VulkanDevice::imageView() {
    return { _device };
}

VulkanBufferCreator VulkanDevice::buffer() {
    return { _allocator };
}

VulkanDevice::operator VkDevice() const {
    return _device;
}

void VulkanDevice::wait() {
    vkDeviceWaitIdle(_device);
}

VulkanShaderModuleCreator VulkanDevice::shader() {
    return { _device };
}

VulkanDescriptorPoolCreator VulkanDevice::descriptorPool() {
    return { _device };
}

VulkanPipelineDescriptorSetLayoutCreator VulkanDevice::descriptorSetLayout() {
    return { _device };
}

VulkanPipelineLayoutCreator VulkanDevice::pipelineLayout() {
    return { _device };
}

std::unique_ptr<VulkanPipeline> VulkanDevice::graphicsPipeline(VkGraphicsPipelineCreateInfo createInfo) {
    VkPipeline pipeline;
    auto result =  vkCreateGraphicsPipelines(_device, nullptr, 1, &createInfo, nullptr, &pipeline);
    if(result != VK_SUCCESS) {
        throw std::runtime_error{ "unable to create graphics pipeline" };
    }
    return std::make_unique<VulkanPipeline>(_device, pipeline);
}

VulkanDeviceBuilder::VulkanDeviceBuilder(VkInstance instance)
: _instance(instance){}

VulkanDeviceBuilder& VulkanDeviceBuilder::addSurface(VkSurfaceKHR surface) {
    _surface = surface;
    return *this;
}

VulkanDeviceBuilder &VulkanDeviceBuilder::addQueue(VkQueueFlagBits queueType) {
     _queueTypes |= queueType;
    return *this;
}

VulkanDeviceBuilder &VulkanDeviceBuilder::addUniqueQueue(VkQueueFlagBits queueType) {
    _uniqueQueueTypes |= queueType;

    return *this;
}


VulkanDeviceBuilder &VulkanDeviceBuilder::addLayer(const char *layer) {
    _enabledLayers.push_back(layer);
    return *this;
}

VulkanDeviceBuilder &VulkanDeviceBuilder::addExtension(const char *extension) {
    _enabledExtensions.push_back(extension);
    return *this;
}

VulkanDeviceBuilder &VulkanDeviceBuilder::addLayers(std::vector<const char *> layers) {
    _enabledLayers.insert(_enabledLayers.end(), layers.begin(), layers.end());
    return *this;
}

VulkanDeviceBuilder &VulkanDeviceBuilder::addExtensions(std::vector<const char *> extensions) {
    _enabledExtensions.insert(_enabledExtensions.end(), extensions.begin(), extensions.end());
    return *this;
}

VkPhysicalDevice VulkanDeviceBuilder::pickDevice(VulkanDeviceBuilder::DevicePicker &&pick) {
    auto physicalDevices = v_enumerate<VkPhysicalDevice>(vkEnumeratePhysicalDevices, _instance);
    _physicalDevice = pick(physicalDevices);
    return _physicalDevice;
}

std::shared_ptr<VulkanDevice> VulkanDeviceBuilder::make_shared() {
    if(!_physicalDevice) {
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
    return std::make_shared<VulkanDevice>(_instance, _physicalDevice, device, allocator, queueFamilyIndex, queues);
}

std::map<VkQueueFlags, uint32> VulkanDeviceBuilder::getQueueFamilyIndexes() {
    assert(_queueTypes != 0 && "queueTypes not set");
    const auto props = v_enumerate<VkQueueFamilyProperties>(vkGetPhysicalDeviceQueueFamilyProperties, _physicalDevice);

    std::map<VkFlags, uint32> queueFamilyIndex;

    auto queryQueue = [&, queueType = _queueTypes](auto queueFamily, auto queueFlagBits, auto qfIndex){
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

        if(_surface && !queueFamilyIndex.contains(VK_QUEUE_PRESENT_BIT)){
            VkBool32 presentSupported;
            vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, _surface, &presentSupported);
            if(presentSupported){
                queueFamilyIndex[VK_QUEUE_PRESENT_BIT] = i;
            }
        }
    }



    auto queryUniqueQueue =
            [&, queueType = _uniqueQueueTypes, previousQueueFamily = std::map<VkFlags, uint32>{}]
            (auto queueFamily, auto queueFlagBits, auto qfIndex) mutable  {
                auto countBits = [](auto bitset){
                    auto size = sizeof(bitset) * 8;
                    auto sum = 0;
                    for(auto i = 0; i < size; ++i){
                        sum += (bitset >> i) & 1;
                    }
                    return sum;
                };

                auto count = countBits(queueFamily.queueFlags);
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
    createInfo.enabledLayerCount = to<uint32>(_enabledLayers.size());
    createInfo.ppEnabledLayerNames = _enabledLayers.data();
    createInfo.enabledExtensionCount = to<uint32>(_enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = _enabledExtensions.data();
    createInfo.pEnabledFeatures = &_enabledFeatures;

    VkDevice device;
    auto result = vkCreateDevice(_physicalDevice, &createInfo, VK_NULL_HANDLE, &device);

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
    allocatorCreateInfo.physicalDevice = _physicalDevice;
    allocatorCreateInfo.device = device;
    allocatorCreateInfo.instance = _instance;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    VmaAllocator allocator;
    vmaCreateAllocator(&allocatorCreateInfo, &allocator);
    return allocator;
}