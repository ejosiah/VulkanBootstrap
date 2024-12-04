#pragma once

#include <volk.h>

#include <string>
#include <vector>
#include <memory>
#include "WindowInterface.hpp"


class VulkanInstanceBuilder;

class VulkanInstance{
public:
    VulkanInstance(const VkInstanceCreateInfo& createInfo, VkInstance instance);

    ~VulkanInstance();

    void set(VkSurfaceKHR surface);

    VkSurfaceKHR surface() const;

public:
    static VulkanInstanceBuilder builder();

    VkInstance handle() const;

private:
    VkApplicationInfo appInfo_;
    VkInstanceCreateInfo createInfo_{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    VkInstance instance_{};
    VkSurfaceKHR surface_{};
};

class VulkanInstanceBuilder{
public:
    VulkanInstanceBuilder() = default;

    VulkanInstanceBuilder& appName(std::string_view value);
    VulkanInstanceBuilder& appVersion(uint32_t version);
    VulkanInstanceBuilder& engineName(std::string_view value);
    VulkanInstanceBuilder& engineVersion(uint32_t version);
    VulkanInstanceBuilder& vulkanVersion(uint32_t version);

    VulkanInstanceBuilder& addLayer(const char* layer);
    VulkanInstanceBuilder& addExtension(const char* extension);

    VulkanInstanceBuilder& addExtensions(std::vector<const char*> extension);

    VulkanInstanceBuilder& addWindow(std::shared_ptr<Window> window);

    std::shared_ptr<VulkanInstance> make_shared();

private:
    std::string appName_{"Undefined"};
    uint32_t appVersion_{VK_MAKE_API_VERSION(0, 0, 0, 0)};
    std::string engineName_{"Undefined"};
    uint32_t engineVersion_{0};
    uint32_t apiVersion_{VK_API_VERSION_1_3};

    std::vector<const char*> enabledLayers_;
    std::vector<const char*> enabledExtensions_;
    std::shared_ptr<Window> window_;
};