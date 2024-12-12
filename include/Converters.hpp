#pragma once

#include <volk.h>
#include <string>

namespace format {
    int toNumChannels(VkFormat format);

    std::string toString(VkFormat format);
}