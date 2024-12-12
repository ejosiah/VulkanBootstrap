#pragma once

#include "Types.hpp"
#include <filesystem>
#include <vector>

namespace io {
    using ByteString = std::vector<char>;

    struct Image {
        int width{1};
        int height{1};
        int channels{4};
        size_t size;
        ByteString data;
    };

    ByteString loadFile(const std::filesystem::path &path);

    Image loadImage(const std::filesystem::path &path, int numChannels = 4, bool flip = true);
}