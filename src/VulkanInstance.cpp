#include "VulkanInstance.hpp"
#include "VulkanDebugMessenger.hpp"

#include <spdlog/spdlog.h>
#include <iostream>



VulkanInstance::VulkanInstance(const VkInstanceCreateInfo &createInfo, VkInstance instance)
: _createInfo(createInfo)
, _appInfo(*createInfo.pApplicationInfo)
, _instance(instance)
{}

VulkanInstance::~VulkanInstance() {
    vkDestroyInstance(_instance, VK_NULL_HANDLE);
}

VulkanInstanceBuilder VulkanInstance::builder() {
    return {};
}

VkInstance VulkanInstance::handle() const {
    return _instance;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::appName(std::string_view value) {
    _appName = value;
    return *this;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::appVersion(uint32_t version) {
    _appVersion = version;
    return *this;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::engineName(std::string_view value) {
    _engineName = value;
    return *this;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::engineVersion(uint32_t version) {
    _engineVersion = version;
    return *this;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::vulkanVersion(uint32_t version) {
    _apiVersion = version;
    return *this;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::addLayer(const char* layer) {
    _enabledLayers.push_back(layer);
    return *this;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::addExtension(const char* extension) {
    _enabledExtensions.push_back(extension);
    return *this;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::addExtensions(std::vector<const char *> extension) {
    _enabledExtensions.insert(_enabledExtensions.end(), extension.begin(), extension.end());
    return *this;
}

std::shared_ptr<VulkanInstance> VulkanInstanceBuilder::make_shared() {
    VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.pApplicationName = _appName.c_str();
    appInfo.applicationVersion = _appVersion;
    appInfo.pEngineName = _engineName.c_str();
    appInfo.engineVersion = _engineVersion;
    appInfo.apiVersion = _appVersion;

#ifndef NDEBUG
    _enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    createInfo.pApplicationInfo = &appInfo;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(_enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = _enabledExtensions.data();

    createInfo.enabledLayerCount = static_cast<uint32_t>(_enabledLayers.size());
    createInfo.ppEnabledLayerNames = _enabledLayers.data();

#ifndef NDEBUG
    auto debugInfo = VulkanDebugMessenger::debugCreateInfo();
    createInfo.pNext = &debugInfo;
#endif
    VkInstance instance{};

    auto result = vkCreateInstance(&createInfo, VK_NULL_HANDLE, &instance);


    if(result != VK_SUCCESS){
        return {};
    }

    return std::make_shared<VulkanInstance>(createInfo, instance);
}
