#pragma once

#include <volk.h>

template<typename Referer>
class VulkanEnabledFeatures {
public:

    Referer& back() const {
        return *referer;
    }

    [[nodiscard]] VkPhysicalDeviceFeatures get() const {
        return _enabledFeatures;
    }

    VulkanEnabledFeatures& enableGeometryShader() {
        _enabledFeatures.geometryShader = VK_TRUE;
        return *this;
    }


private:
    VkPhysicalDeviceFeatures _enabledFeatures{};
    Referer* referer;
};