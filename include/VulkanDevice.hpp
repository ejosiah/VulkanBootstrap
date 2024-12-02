#pragma once

#include "VulkanCommandPool.hpp"
#include "VulkanImage.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanPipeline.hpp"

#include <volk.h>
#include <vk_mem_alloc.h>

#include <vector>
#include <string>
#include <map>
#include <set>
#include <memory>
#include <functional>
#include <span>

class VulkanDeviceBuilder;

enum VkQueueFlagBitsExtras {
    VK_QUEUE_PRESENT_BIT = VK_QUEUE_FLAG_BITS_MAX_ENUM >> 1
};

class VulkanDevice {
public:
    friend class VulkanDeviceBuilder;
    VulkanDevice(
        VkInstance instance,
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VmaAllocator allocator,
        std::map<VkQueueFlags, uint32_t> queueFamilyIndex,
        std::map<VkQueueFlags, VkQueue> queues);

    ~VulkanDevice();

    operator VkDevice() const;

    std::string toString();

    static VulkanDeviceBuilder builder(VkInstance instance);

    static std::string toString(VkPhysicalDeviceType deviceType);

    [[nodiscard]] VkQueue getGraphicsQueue() const;

    [[nodiscard]] VkQueue getComputeQueue() const;

    [[nodiscard]] VkQueue getTransferQueue() const;

    [[nodiscard]] VkQueue getPresentQueue() const;

    [[nodiscard]] VkQueue getQueue(uint32_t queueFamilyIndex) const;

    [[nodiscard]] VkSurfaceCapabilitiesKHR getSurfaceCapabilities(VkSurfaceKHR surface) const;

    [[nodiscard]] std::vector<VkSurfaceFormatKHR> getSurfaceFormat(VkSurfaceKHR surface) const;

    [[nodiscard]] std::vector<VkPresentModeKHR> getSurfacePresentationsModes(VkSurfaceKHR surface) const;

    [[nodiscard]] VkDevice handle() const;

    std::shared_ptr<VulkanCommandPool> createCommandPool(VkQueueFlagBits queueFlag
                                           , VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    VulkanImageCreator image();

    VulkanImageViewCreator imageView();

    VulkanBufferCreator buffer();

    VulkanShaderModuleCreator shader();

    VulkanDescriptorPoolCreator descriptorPool();

    VulkanPipelineDescriptorSetLayoutCreator descriptorSetLayout();

    VulkanPipelineLayoutCreator pipelineLayout();

    std::unique_ptr<VulkanPipeline> graphicsPipeline(VkGraphicsPipelineCreateInfo createInfo);

    void wait();

private:
    VkInstance _instance;
    VkPhysicalDevice _physicalDevice;
    VkDevice _device;
    VmaAllocator _allocator;
    std::map<VkQueueFlags, uint32_t> _queueFamilyIndex;
    std::map<VkQueueFlags, VkQueue> _queues;
};

class VulkanDeviceBuilder {
public:
    using DevicePicker = std::function<VkPhysicalDevice(std::span<VkPhysicalDevice>)>;

    VulkanDeviceBuilder(VkInstance instance);

    VulkanDeviceBuilder& addSurface(VkSurfaceKHR surface);

    VulkanDeviceBuilder& addQueue(VkQueueFlagBits queueType);

    VulkanDeviceBuilder& addUniqueQueue(VkQueueFlagBits queueType);

    VulkanDeviceBuilder& addLayer(const char* layer);
    VulkanDeviceBuilder& addExtension(const char* extension);

    VulkanDeviceBuilder& addLayers(std::vector<const char*> layers);
    VulkanDeviceBuilder& addExtensions(std::vector<const char*> extensions);

    VkPhysicalDevice pickDevice(DevicePicker&& picker = [](std::span<VkPhysicalDevice> devices){ return devices.front(); });

    std::shared_ptr<VulkanDevice> make_shared();

private:
    std::map<VkQueueFlags, uint32_t> getQueueFamilyIndexes();

    VkDevice createDevice(const std::map<VkQueueFlags, uint32_t>& queueFamilyIndexes);

    VmaAllocator createAllocator(VkDevice device);

private:

    VkInstance _instance;
    std::vector<const char*> _enabledLayers;
    std::vector<const char*> _enabledExtensions;
    VkQueueFlags _queueTypes{0};
    VkQueueFlags _uniqueQueueTypes{0};
    VkSurfaceKHR _surface{};
    VkPhysicalDevice _physicalDevice{};
    VkPhysicalDeviceFeatures _enabledFeatures{ };

};