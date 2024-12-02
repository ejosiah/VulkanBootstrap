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
    VkDevice _device;
    VkShaderModule _module;
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
    VkDevice _device;
     Code _code;
    VkShaderModuleCreateInfo _info{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
};