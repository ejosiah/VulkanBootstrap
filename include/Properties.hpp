#pragma once

#include "Types.hpp"
#include <volk.h>

struct Properties {
    uint32 width{};
    uint32 height{};
    uint32_t colorAttachmentCount{1};
    uint32_t framesInFlight{};
    VkFormat colorFormat;
    VkFormat depthFormat;
    VkSampleCountFlagBits samples{VK_SAMPLE_COUNT_1_BIT};
    uint32 currentFrame{};
};