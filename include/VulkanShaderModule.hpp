#pragma once

#include "Types.hpp"
#include <volk.h>

#include <variant>
#include <vector>
#include <string>
#include <memory>
#include <filesystem>

class VulkanShaderModule {
public:
    VulkanShaderModule(VkDevice device, VkShaderModule module);

    ~VulkanShaderModule();

    operator VkShaderModule();

private:
    VkDevice device_;
    VkShaderModule module_;
};

class VulkanShaderModuleCreator {
    using Code = std::variant<std::vector<uint8>, std::vector<uint32>, std::filesystem::path>;
public:
    VulkanShaderModuleCreator(VkDevice device);

    VulkanShaderModuleCreator& flag(VkShaderModuleCreateFlags flags);

    VulkanShaderModuleCreator& code(Code code);

    std::shared_ptr<VulkanShaderModule> make_shared();

private:
    std::vector<uint32> getCode();

private:
    VkDevice device_;
    Code code_;
    VkShaderModuleCreateInfo info_{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
};