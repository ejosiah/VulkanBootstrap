#pragma once

#include <volk.h>
#include <memory>

class VulkanDebugMessenger{
public:
    VulkanDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger);

    ~VulkanDebugMessenger();

    static std::shared_ptr<VulkanDebugMessenger> createDebugMessenger(VkInstance instance);

    static VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo();


private:
    static  VkBool32 VKAPI_PTR debugCallBack(
            VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
            const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
            void*                                            pUserData);

private:
    VkDebugUtilsMessengerEXT debugMessenger_{};
    VkInstance instance_;
};