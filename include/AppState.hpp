#pragma once

#include "Types.hpp"
#include <volk.h>

struct AppState {
    static uint32 screenWidth;
    static uint32 screenHeight;
    static uint32 colorBufferCount;
    static VkFormat screenFormat;
    static uint32 numFramesInFlight;
    static VkFormat screenDepthFormat;
    static VkSampleCountFlagBits screenSampleCount;
    static uint32 currentFrame;

    static float ScreenAspectRatio() {
        return to<float>(screenWidth)/to<float>(screenHeight);
    }
};