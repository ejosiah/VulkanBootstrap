#pragma once

#include "VulkanCommandPool.hpp"
#include "VulkanImage.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanPipeline.hpp"
#include "SetVulkanObjectName.hpp"

#include <volk.h>
#include <vk_mem_alloc.h>

#include <vector>
#include <string>
#include <map>
#include <set>
#include <memory>
#include <functional>
#include <span>
#include <string_view>

class VulkanDeviceBuilder;

enum VkQueueFlagBitsExtras {
    VK_QUEUE_PRESENT_BIT = VK_QUEUE_FLAG_BITS_MAX_ENUM >> 1
};

class VulkanDevice {
public:
    friend class VulkanDeviceBuilder;
    friend class TextureBuilder;
    friend class TextureImageCreator;
    friend class TextureSamplerCreator;
    VulkanDevice(
        VkInstance instance,
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VmaAllocator allocator,
        std::map<VkQueueFlags, uint32_t> queueFamilyIndex,
        std::map<VkQueueFlags, VkQueue> queues,
        std::unique_ptr<VulkanCommandPool> graphicsCommandPool);

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

    [[nodiscard]] VkPhysicalDeviceProperties getProperties() const;

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

    VulkanCommandPool* graphicsCommandPool();

    template<typename Object>
    void setName(Object object, const std::string& name);

    TextureBuilder texture();

private:
    VkInstance instance_;
    VkPhysicalDevice physicalDevice_;
    VkDevice device_;
    VmaAllocator allocator_;
    std::map<VkQueueFlags, uint32_t> queueFamilyIndex_;
    std::map<VkQueueFlags, VkQueue> queues_;
    std::unique_ptr<VulkanCommandPool> graphicsCommandPool_;
};

using VulkanDevicePtr = std::unique_ptr<VulkanDevice>;
using VulkanDeviceSptr = std::shared_ptr<VulkanDevice>;

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

    VulkanDeviceSptr make_shared();

private:
    std::map<VkQueueFlags, uint32_t> getQueueFamilyIndexes();

    VkDevice createDevice(const std::map<VkQueueFlags, uint32_t>& queueFamilyIndexes);

    VmaAllocator createAllocator(VkDevice device);

private:

    VkInstance instance_;
    std::vector<const char*> enabledLayers_;
    std::vector<const char*> enabledExtensions_;
    VkQueueFlags queueTypes_{0};
    VkQueueFlags uniqueQueueTypes_{0};
    VkSurfaceKHR surface_{};
    VkPhysicalDevice physicalDevice_{};
    VkPhysicalDeviceFeatures enabledFeatures_{ };

};

template<typename Object>
void VulkanDevice::setName(Object object, const std::string &name) {
    if constexpr (std::is_pointer_v<Object>) {
        setVulkanObjectName(device_, object->objectType, object->handle(), name);
    }else {
        setVulkanObjectName(device_, object.objectType, object.handle(), name);
    }
}


