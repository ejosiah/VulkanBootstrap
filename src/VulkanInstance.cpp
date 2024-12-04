#include "Types.hpp"
#include "VulkanInstance.hpp"
#include "VulkanDebugMessenger.hpp"
#include "WindowInterface.hpp"
#include <spdlog/spdlog.h>
#include <iostream>
#include <utility>



VulkanInstance::VulkanInstance(const VkInstanceCreateInfo &createInfo, VkInstance instance)
: createInfo_(createInfo)
, appInfo_(*createInfo.pApplicationInfo)
, instance_(instance)
{}

VulkanInstance::~VulkanInstance() {
    if(surface_){
       vkDestroySurfaceKHR(instance_, surface_, nullptr);
    }
    vkDestroyInstance(instance_, VK_NULL_HANDLE);
}

VulkanInstanceBuilder VulkanInstance::builder() {
    return {};
}

VkInstance VulkanInstance::handle() const {
    return instance_;
}

void VulkanInstance::set(VkSurfaceKHR surface) {
    surface_ = surface;
}

VkSurfaceKHR VulkanInstance::surface() const {
    return surface_;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::appName(std::string_view value) {
    appName_ = value;
    return *this;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::appVersion(uint32 version) {
    appVersion_ = version;
    return *this;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::engineName(std::string_view value) {
    engineName_ = value;
    return *this;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::engineVersion(uint32 version) {
    engineVersion_ = version;
    return *this;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::vulkanVersion(uint32 version) {
    apiVersion_ = version;
    return *this;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::addLayer(const char* layer) {
    enabledLayers_.push_back(layer);
    return *this;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::addExtension(const char* extension) {
    enabledExtensions_.push_back(extension);
    return *this;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::addExtensions(std::vector<const char *> extension) {
    enabledExtensions_.insert(enabledExtensions_.end(), extension.begin(), extension.end());
    return *this;
}

std::shared_ptr<VulkanInstance> VulkanInstanceBuilder::make_shared() {
    VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.pApplicationName = appName_.c_str();
    appInfo.applicationVersion = appVersion_;
    appInfo.pEngineName = engineName_.c_str();
    appInfo.engineVersion = engineVersion_;
    appInfo.apiVersion = apiVersion_;

#ifndef NDEBUG
    enabledExtensions_.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    createInfo.pApplicationInfo = &appInfo;

    createInfo.enabledExtensionCount = to<uint32>(enabledExtensions_.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions_.data();

    createInfo.enabledLayerCount = to<uint32>(enabledLayers_.size());
    createInfo.ppEnabledLayerNames = enabledLayers_.data();

#ifndef NDEBUG
    auto debugInfo = VulkanDebugMessenger::debugCreateInfo();
    createInfo.pNext = &debugInfo;
#endif
    VkInstance instance{};

    auto result = vkCreateInstance(&createInfo, VK_NULL_HANDLE, &instance);


    if(result != VK_SUCCESS){
        return {};
    }

    auto appInstance = std::make_shared<VulkanInstance>(createInfo, instance);
    if(window_){
        appInstance->set(window_->createSurface(instance));
    }
    return appInstance;
}

VulkanInstanceBuilder &VulkanInstanceBuilder::addWindow(std::shared_ptr<Window> window) {
    addExtensions(WindowInterface::extensions());
    window_ = std::move(window);
    return *this;
}
