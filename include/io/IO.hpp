#pragma once

#include <filesystem>
#include <vector>

using ByteString = std::vector<char>;

struct IO {
    static ByteString loadFile(const std::filesystem::path& path);

private:
    IO() = default;
};