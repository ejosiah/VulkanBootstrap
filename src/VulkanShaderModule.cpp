#include "VulkanShaderModule.hpp"
#include "io/IO.hpp"

#include <utility>
#include <stdexcept>

VulkanShaderModule::VulkanShaderModule(VkDevice device, VkShaderModule module)
: _device(device)
, _module(module){}

VulkanShaderModule::~VulkanShaderModule() {
    vkDestroyShaderModule(_device, _module, VK_NULL_HANDLE);
}

VulkanShaderModule::operator VkShaderModule() {
    return _module;
}

VulkanShaderModuleCreator::VulkanShaderModuleCreator(VkDevice device)
: _device(device){}

VulkanShaderModuleCreator &VulkanShaderModuleCreator::flag(VkShaderModuleCreateFlags flags) {
    _info.flags = flags;
    return *this;
}

VulkanShaderModuleCreator &VulkanShaderModuleCreator::code(VulkanShaderModuleCreator::Code code) {
    _code = std::move(code);
    return *this;
}

std::shared_ptr<VulkanShaderModule> VulkanShaderModuleCreator::make_shared() {
    auto code = getCode();
    _info.codeSize = code.size() * sizeof(uint32_t);
    _info.pCode = code.data();

    VkShaderModule module;
    auto result = vkCreateShaderModule(_device, &_info, VK_NULL_HANDLE, &module);
    if(result != VK_SUCCESS){
        throw std::runtime_error{ "unable to create shader module"};
    }
    return std::make_shared<VulkanShaderModule>(_device, module);
}



std::vector<uint32> VulkanShaderModuleCreator::getCode() {
    return std::visit(
        overloaded(
            [](const std::filesystem::path& path){
                auto code = IO::loadFile(path);
                std::vector<uint32> codeU32(code.size()/sizeof(uint32));
                std::memcpy(codeU32.data(), code.data(), code.size());
                return codeU32;
            },
            [](const std::vector<uint32_t>& code) { return code; },
            [](const std::vector<uint8>& code){ return std::vector<uint32>{}; })
        , _code);
}
