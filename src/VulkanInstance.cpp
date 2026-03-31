#include "Types.hpp"
#include "VulkanInstance.hpp"
#include "VulkanDebugMessenger.hpp"
#include "WindowInterface.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace {
std::vector<VkExtensionProperties> enumerateInstanceExtensions() {
    uint32 count = 0;
    auto result = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    if(result != VK_SUCCESS) {
        spdlog::error("failed to enumerate Vulkan instance extensions, vk result={}", static_cast<int>(result));
        throw std::runtime_error("failed to enumerate Vulkan instance extensions");
    }

    std::vector<VkExtensionProperties> extensions(count);
    result = vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
    if(result != VK_SUCCESS) {
        spdlog::error("failed to read Vulkan instance extensions, vk result={}", static_cast<int>(result));
        throw std::runtime_error("failed to read Vulkan instance extensions");
    }

    extensions.resize(count);
    return extensions;
}

std::vector<VkLayerProperties> enumerateInstanceLayers() {
    uint32 count = 0;
    auto result = vkEnumerateInstanceLayerProperties(&count, nullptr);
    if(result != VK_SUCCESS) {
        spdlog::error("failed to enumerate Vulkan instance layers, vk result={}", static_cast<int>(result));
        throw std::runtime_error("failed to enumerate Vulkan instance layers");
    }

    std::vector<VkLayerProperties> layers(count);
    result = vkEnumerateInstanceLayerProperties(&count, layers.data());
    if(result != VK_SUCCESS) {
        spdlog::error("failed to read Vulkan instance layers, vk result={}", static_cast<int>(result));
        throw std::runtime_error("failed to read Vulkan instance layers");
    }

    layers.resize(count);
    return layers;
}

bool hasExtension(const std::vector<VkExtensionProperties>& extensions, const char* extensionName) {
    return std::any_of(extensions.begin(), extensions.end(), [extensionName](const auto& extension) {
        return std::strcmp(extension.extensionName, extensionName) == 0;
    });
}

bool hasLayer(const std::vector<VkLayerProperties>& layers, const char* layerName) {
    return std::any_of(layers.begin(), layers.end(), [layerName](const auto& layer) {
        return std::strcmp(layer.layerName, layerName) == 0;
    });
}
}

VulkanInstance::VulkanInstance(const VkInstanceCreateInfo &createInfo,
                               VkApplicationInfo appInfo,
                               std::vector<const char*> enabledLayers,
                               std::vector<const char*> enabledExtensions,
                               VkInstance instance)
: appInfo_(appInfo)
, createInfo_(createInfo)
, enabledLayers_(std::move(enabledLayers))
, enabledExtensions_(std::move(enabledExtensions))
, instance_(instance) {
    createInfo_.pApplicationInfo = &appInfo_;
    createInfo_.enabledLayerCount = to<uint32>(enabledLayers_.size());
    createInfo_.ppEnabledLayerNames = enabledLayers_.data();
    createInfo_.enabledExtensionCount = to<uint32>(enabledExtensions_.size());
    createInfo_.ppEnabledExtensionNames = enabledExtensions_.data();
}

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
    auto enabledExtensions = enabledExtensions_;
    auto enabledLayers = enabledLayers_;
    const auto availableExtensions = enumerateInstanceExtensions();
    const auto availableLayers = enumerateInstanceLayers();

    for(const auto* extension : enabledExtensions) {
        spdlog::info("Requesting Vulkan instance extension: {}", extension);
    }

    for(const auto* layer : enabledLayers) {
        spdlog::info("Requesting Vulkan instance layer: {}", layer);
    }

#ifndef NDEBUG
    const bool debugUtilsSupported = hasExtension(availableExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    if(debugUtilsSupported &&
       std::find(enabledExtensions.begin(), enabledExtensions.end(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == enabledExtensions.end()) {
        enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
#endif

    for(const auto* extension : enabledExtensions) {
        if(!hasExtension(availableExtensions, extension)) {
            spdlog::error("required Vulkan instance extension is unavailable: {}", extension);
            throw std::runtime_error("required Vulkan instance extension is unavailable");
        }
    }

    for(const auto* layer : enabledLayers) {
        if(!hasLayer(availableLayers, layer)) {
            spdlog::error("required Vulkan instance layer is unavailable: {}", layer);
            throw std::runtime_error("required Vulkan instance layer is unavailable");
        }
    }

    VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.pApplicationName = appName_.c_str();
    appInfo.applicationVersion = appVersion_;
    appInfo.pEngineName = engineName_.c_str();
    appInfo.engineVersion = engineVersion_;
    const auto loaderApiVersion = volkGetInstanceVersion();
    appInfo.apiVersion = loaderApiVersion != 0 ? std::min(apiVersion_, loaderApiVersion) : apiVersion_;
    spdlog::info("Using Vulkan API version {}.{}.{}",
                 VK_API_VERSION_MAJOR(appInfo.apiVersion),
                 VK_API_VERSION_MINOR(appInfo.apiVersion),
                 VK_API_VERSION_PATCH(appInfo.apiVersion));

    VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    createInfo.pApplicationInfo = &appInfo;

    createInfo.enabledExtensionCount = to<uint32>(enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions.empty() ? nullptr : enabledExtensions.data();

    createInfo.enabledLayerCount = to<uint32>(enabledLayers.size());
    createInfo.ppEnabledLayerNames = enabledLayers.empty() ? nullptr : enabledLayers.data();

#ifndef NDEBUG
    VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
    if(debugUtilsSupported) {
        debugInfo = VulkanDebugMessenger::debugCreateInfo();
        createInfo.pNext = &debugInfo;
    }
#endif
    VkInstance instance{};

    auto result = vkCreateInstance(&createInfo, VK_NULL_HANDLE, &instance);
    if(result != VK_SUCCESS){
        spdlog::error("unable to create Vulkan instance, vk result={}", static_cast<int>(result));
        throw std::runtime_error("unable to create Vulkan instance");
    }

    auto appInstance = std::make_shared<VulkanInstance>(createInfo, appInfo, std::move(enabledLayers), std::move(enabledExtensions), instance);
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
