#pragma once

#include "Types.hpp"
#include <volk.h>

struct AppState {
    friend class VulkanApplication;

    [[nodiscard]] float ScreenAspectRatio() const {
        return to<float>(screenWidth_) / to<float>(screenHeight_);
    }

    static AppState &instance() {
        static AppState state{};
        return state;
    }

    [[nodiscard]] uint32 screenWidth() const {
        return screenWidth_;
    }

    [[nodiscard]] uint32 screenHeight() const {
        return screenHeight_;
    }

    [[nodiscard]] uint32 colorBufferCount() const {
        return colorBufferCount_;
    }

    [[nodiscard]] const VkFormat& screenFormat() const {
        return screenFormat_;
    }

    [[nodiscard]] uint32 numFramesInFlight() const {
        return numFramesInFlight_;
    }

    [[nodiscard]] VkFormat screenDepthFormat() const {
        return screenDepthFormat_;
    }

    [[nodiscard]] VkSampleCountFlagBits screenSampleCount() const {
        return screenSampleCount_;
    }

    [[nodiscard]] uint32 currentFrame() const {
        return currentFrame_;
    }

private:
    uint32 screenWidth_;
    uint32 screenHeight_;
    uint32 colorBufferCount_;
    VkFormat screenFormat_;
    uint32 numFramesInFlight_;
    VkFormat screenDepthFormat_;
    VkSampleCountFlagBits screenSampleCount_;
    uint32 currentFrame_;
};