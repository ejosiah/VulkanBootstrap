#include "VulkanShaderModule.hpp"
#include "io/IO.hpp"

#include <cstring>
#include <utility>
#include <stdexcept>

VulkanShaderModule::VulkanShaderModule(VkDevice device, VkShaderModule module)
: device_(device)
, module_(module){}

VulkanShaderModule::~VulkanShaderModule() {
    vkDestroyShaderModule(device_, module_, VK_NULL_HANDLE);
}

VulkanShaderModule::operator VkShaderModule() {
    return module_;
}

VulkanShaderModuleCreator::VulkanShaderModuleCreator(VkDevice device)
: device_(device){}

VulkanShaderModuleCreator &VulkanShaderModuleCreator::flag(VkShaderModuleCreateFlags flags) {
    info_.flags = flags;
    return *this;
}

VulkanShaderModuleCreator &VulkanShaderModuleCreator::code(VulkanShaderModuleCreator::Code code) {
    code_ = std::move(code);
    return *this;
}

std::shared_ptr<VulkanShaderModule> VulkanShaderModuleCreator::make_shared() {
    auto code = getCode();
    info_.codeSize = code.size() * sizeof(uint32_t);
    info_.pCode = code.data();

    VkShaderModule module;
    auto result = vkCreateShaderModule(device_, &info_, VK_NULL_HANDLE, &module);
    if(result != VK_SUCCESS){
        throw std::runtime_error{ "unable to create shader module"};
    }
    return std::make_shared<VulkanShaderModule>(device_, module);
}



std::vector<uint32> VulkanShaderModuleCreator::getCode() {
    return std::visit(
        overloaded(
            [](const std::filesystem::path& path){
                auto code = io::loadFile(path);
                std::vector<uint32> codeU32(code.size()/sizeof(uint32));
                std::memcpy(codeU32.data(), code.data(), code.size());
                return codeU32;
            },
            [](const std::vector<uint32_t>& code) { return code; },
            [](const std::vector<uint8>& code){ return std::vector<uint32>{}; })
        , code_);
}
