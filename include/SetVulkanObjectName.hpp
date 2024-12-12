#pragma once

#include "Types.hpp"
#include <volk.h>
#include <string_view>


inline void setVulkanObjectName(VkDevice device, VkObjectType ObjectType, auto object, std::string_view name) {
#ifndef NDEBUG
    VkDebugUtilsObjectNameInfoEXT info{ VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
    info.objectType = ObjectType;
    info.objectHandle = (uint64)(object);
    info.pObjectName = name.data();
    vkSetDebugUtilsObjectNameEXT(device, &info);
#endif
}
