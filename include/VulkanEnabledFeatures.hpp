#pragma once

#include <volk.h>

template<typename Referer>
class VulkanEnabledFeatures {
public:

    Referer& back() const {
        return *referer;
    }

    [[nodiscard]] VkPhysicalDeviceFeatures get() const {
        return enabledFeatures_;
    }

    VulkanEnabledFeatures& enableGeometryShader() {
        enabledFeatures_.geometryShader = VK_TRUE;
        return *this;
    }


private:
    VkPhysicalDeviceFeatures enabledFeatures_{};
    Referer* referer;
};