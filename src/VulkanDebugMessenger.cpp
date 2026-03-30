#include "VulkanDebugMessenger.hpp"

#include <spdlog/spdlog.h>

VulkanDebugMessenger::VulkanDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger)
:instance_(instance)
, debugMessenger_(debugMessenger)
{}

VulkanDebugMessenger::~VulkanDebugMessenger() {
    vkDestroyDebugUtilsMessengerEXT(instance_, debugMessenger_, VK_NULL_HANDLE);
}

VkBool32 VulkanDebugMessenger::debugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                             const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                             void *pUserData) {
    if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT){
        spdlog::debug(pCallbackData->pMessage);
    }

    if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT){
        spdlog::info(pCallbackData->pMessage);
    }
    if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT){
        spdlog::warn(pCallbackData->pMessage);
    }
    if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT){
        spdlog::error(pCallbackData->pMessage);
    }

    return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT VulkanDebugMessenger::debugCreateInfo() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

    createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    createInfo.pfnUserCallback = debugCallBack;

    return createInfo;
}

std::shared_ptr <VulkanDebugMessenger> VulkanDebugMessenger::createDebugMessenger(VkInstance instance) {
#ifdef NDEBUG
    return {};
#endif
    auto createInfo = debugCreateInfo();

    VkDebugUtilsMessengerEXT messenger{};
    auto result = vkCreateDebugUtilsMessengerEXT(instance, &createInfo, VK_NULL_HANDLE, &messenger);

    if(result != VK_SUCCESS){
        return {};
    }

    return std::make_shared<VulkanDebugMessenger>(instance, messenger);
}
