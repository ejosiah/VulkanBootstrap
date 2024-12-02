#include "io/IO.hpp"

#include <stdexcept>
#include <format>
#include <fstream>

ByteString IO::loadFile(const std::filesystem::path &path) {
    if(!std::filesystem::exists(path)) {
        throw std::runtime_error{ std::format("{} does not exists", path.string()) };
    }

    std::ifstream fin(path.string().data(), std::ios::binary | std::ios::ate);
    if(!fin.good()) throw std::runtime_error{ std::format("Failed to open file: {}", path.string()) };

    auto size = fin.tellg();
    fin.seekg(0);
    ByteString data(size);
    fin.read(data.data(), size);
    return data;
}
