#include "io/IO.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cpptrace/cpptrace.hpp>
#include <format>
#include <fstream>
#include <cstdio>

io::ByteString io::loadFile(const std::filesystem::path &path) {
    if(!std::filesystem::exists(path)) {
        throw cpptrace::runtime_error{ std::format("{} does not exists", path.string()) };
    }

    std::ifstream fin(path.string().data(), std::ios::binary | std::ios::ate);
    if(!fin.good()) throw cpptrace::runtime_error{ std::format("Failed to open file: {}", path.string()) };

    auto size = fin.tellg();
    fin.seekg(0);
    ByteString data(size);
    fin.read(data.data(), size);
    return data;
}

io::Image io::loadImage(const std::filesystem::path &path, int numChannels, bool flip) {
    Image image{};
    stbi_set_flip_vertically_on_load(flip);
    auto pixels = stbi_load(path.string().c_str(), &image.width, &image.height, &image.channels, numChannels);
    image.channels = numChannels;

    if(!pixels) {
        throw cpptrace::runtime_error{ std::format("unable to load image from path: {}", path.string()) };
    }

    image.size = image.width * image.height * numChannels;
    image.data.resize(image.size);
    std::memcpy(image.data.data(), pixels, image.size);
    stbi_image_free(pixels);
    return image;
}