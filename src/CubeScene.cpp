#pragma once

#include "CubeScene.hpp"
#include "Time.hpp"
#include <glm/glm.hpp>
#include <utility>
#include <vector>
#include <cstdio>
#include <spdlog/spdlog.h>

struct Mesh {
    glm::vec3 position{};
    glm::vec3 normal{0, 0, 1};
};

constexpr glm::mat4 GL_TO_VKN_CLIP(1.0f,  0.0f, 0.0f, 0.0f,
                                   0.0f, -1.0f, 0.0f, 0.0f,
                                   0.0f,  0.0f, 0.5f, 0.0f,
                                   0.0f,  0.0f, 0.5f, 1.0f);

std::vector<Mesh> cube{
        // FRONT FACE
        {{-1.0, -1.0, 1.0},  {0.0f,  0.0f,  1.0f}},
        {{1.0,  -1.0, 1.0},  {0.0f,  0.0f,  1.0f}},
        {{1.0,  1.0,  1.0},  {0.0f,  0.0f,  1.0f}},
        {{-1.0, 1.0,  1.0},  {0.0f,  0.0f,  1.0f}},

        // RIGHT FACE
        {{1.0,  -1.0, 1.0},  {1.0f,  0.0f,  0.0f}},
        {{1.0,  -1.0, -1.0}, {1.0f,  0.0f,  0.0f}},
        {{1.0,  1.0,  -1.0}, {1.0f,  0.0f,  0.0f}},
        {{1.0,  1.0,  1.0,}, {1.0f,  0.0f,  0.0f}},

        // BACK FACE
        {{-1.0, -1.0, -1.0}, {0.0f,  0.0f,  -1.0f}},
        {{-1.0, 1.0,  -1.0}, {0.0f,  0.0f,  -1.0f}},
        {{1.0,  1.0,  -1.0}, {0.0f,  0.0f,  -1.0f}},
        {{1.0,  -1.0, -1.0}, {0.0f,  0.0f,  -1.0f}},

        // LEFT FACE
        {{-1.0, -1.0, 1.0,}, {-1.0f, 0.0f,  0.0f}},
        {{-1.0, 1.0,  1.0},  {-1.0f, 0.0f,  0.0f}},
        {{-1.0, 1.0,  -1.0}, {-1.0f, 0.0f,  0.0f}},
        {{-1.0, -1.0, -1.0}, {-1.0f, 0.0f,  0.0f}},

        // BOTTOM FACE
        {{-1.0, -1.0, 1.0,}, {0.0f,  -1.0f, 0.0f}},
        {{-1.0, -1.0, -1.0}, {0.0f,  -1.0f, 0.0f}},
        {{1.0,  -1.0, -1.0}, {0.0f,  -1.0f, 0.0f}},
        {{1.0,  -1.0, 1.0},  {0.0f,  -1.0f, 0.0f}},

        // TOP FACE
        {{-1.0, 1.0,  1.0},  {0.0f,  1.0f,  0.0f}},
        {{1.0,  1.0,  1.0},  {0.0f,  1.0f,  0.0f}},
        {{1.0,  1.0,  -1.0}, {0.0f,  1.0f,  0.0f}},
        {{-1.0, 1.0,  -1.0}, {0.0f,  1.0f,  0.0f}},
};

std::vector<uint16> indices {
        0,1,2,0,2,3,
        4,5,6,4,6,7,
        8,9,10,8,10,11,
        12,13,14,12,14,15,
        16,17,18,16,18,19,
        20,21,22,20,22,23
};

inline glm::mat4 vulkan_perspective(float fovy, float aspect, float zNear, float zFar){
    assert(abs(aspect - std::numeric_limits<float>::epsilon()) > 0);

    float const tanHalfFovy = glm::tan(fovy / 2);

    glm::mat4 Result(0);
    Result[0][0] = 1 / (aspect * tanHalfFovy);
    Result[1][1] = -1 / (tanHalfFovy);
    Result[2][2] = -zFar / (zFar - zNear);
    Result[2][3] = -1;
    Result[3][2] = -(zFar * zNear) / (zFar - zNear);
    return Result;
}

CubeScene::CubeScene(std::shared_ptr<VulkanDevice> device, const AppState& appState)
        : Scene(std::move(device), appState) {}

void CubeScene::init() {
    createBuffers();
    invalidate();
    Events::ClearScreen(0, 0, 0);
}

void CubeScene::createBuffers() {
    auto vByteSize = sizeof(Mesh) * cube.size();
    auto iByteSize = sizeof(uint16) * indices.size();
    auto byteSize = vByteSize + iByteSize;

    cubeVertices_ =
        device_->buffer()
            .memoryUsage(VMA_MEMORY_USAGE_GPU_ONLY)
            .usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            .size(vByteSize)
        .make_shared();

    cubeIndices_ =
        device_->buffer()
            .memoryUsage(VMA_MEMORY_USAGE_GPU_ONLY)
            .usage(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
            .size(iByteSize)
        .make_shared();

    transformBuffer_ =
        device_->buffer()
            .memoryUsage(VMA_MEMORY_USAGE_CPU_TO_GPU)
            .usage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
            .size(sizeof(glm::mat4) * 3)
        .make_shared();

    transform = reinterpret_cast<glm::mat4*>(transformBuffer_->map());

    commandPool_->oneTime([&](auto commandBuffer){
        auto stagingBuffer =
            device_->buffer()
                .memoryUsage(VMA_MEMORY_USAGE_CPU_ONLY)
                .usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
                .size(byteSize)
            .make_shared();

        auto mapping = stagingBuffer->map();

        std::memcpy(mapping, cube.data(), vByteSize);
        VkBufferCopy region{0, 0, vByteSize};
        vkCmdCopyBuffer(commandBuffer, *stagingBuffer, *cubeVertices_, 1, &region);

        std::memcpy(reinterpret_cast<uint8*>(mapping) + vByteSize, indices.data(), iByteSize);
        region.srcOffset = vByteSize;
        region.size = iByteSize;
        vkCmdCopyBuffer(commandBuffer, *stagingBuffer, *cubeIndices_, 1, &region);
    });
}

void CubeScene::createPipeline() {
    auto vertexShader =  device_->shader().code("../../resources/shaders/cube.vert.spv").make_shared();
    auto fragmentShader =  device_->shader().code("../../resources/shaders/cube.frag.spv").make_shared();

    VkPipelineShaderStageCreateInfo vertexStageCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    vertexStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexStageCreateInfo.module = *vertexShader;
    vertexStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragmentStageCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    fragmentStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentStageCreateInfo.module = *fragmentShader;
    fragmentStageCreateInfo.pName = "main";
    std::vector<VkPipelineShaderStageCreateInfo> stages{ vertexStageCreateInfo, fragmentStageCreateInfo};


    VkVertexInputBindingDescription bindingDescription{0, sizeof(glm::vec3) * 2, VK_VERTEX_INPUT_RATE_VERTEX };

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
    attributeDescriptions[0] = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};
    attributeDescriptions[1] = {1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3)};

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineTessellationStateCreateInfo tessellationStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };

    VkViewport viewport{0, 0, to<float>(appState_.screenWidth()), to<float>(appState_.screenHeight()), 0, 1 };
    VkRect2D scissor{ {0, 0}, {appState_.screenWidth(), appState_.screenHeight() }};
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.pViewports = &viewport;
    viewportStateCreateInfo.scissorCount = 1;
    viewportStateCreateInfo.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationStateCreateInfo.lineWidth = 1;

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    multisampleStateCreateInfo.rasterizationSamples = appState_.screenSampleCount();

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
    depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
    depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilStateCreateInfo.minDepthBounds = 0;
    depthStencilStateCreateInfo.maxDepthBounds = 1;

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState{
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_A_BIT
    };
    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };

    pipelineLayout_ =
        device_->pipelineLayout()
            .addSetLayout(*descriptorSetLayout_)
        .make_unique();

    auto colorFormat = appState_.screenFormat();
    VkPipelineRenderingCreateInfo renderingCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    renderingCreateInfo.colorAttachmentCount = 1;
    renderingCreateInfo.pColorAttachmentFormats = &colorFormat;
    renderingCreateInfo.depthAttachmentFormat = appState_.screenDepthFormat();

    VkGraphicsPipelineCreateInfo createInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    createInfo.pNext = &renderingCreateInfo;

    createInfo.stageCount = stages.size();
    createInfo.pStages = stages.data();
    createInfo.pVertexInputState = &vertexInputStateCreateInfo;
    createInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
    createInfo.pTessellationState = &tessellationStateCreateInfo;
    createInfo.pViewportState = &viewportStateCreateInfo;
    createInfo.pRasterizationState = &rasterizationStateCreateInfo;
    createInfo.pMultisampleState = &multisampleStateCreateInfo;
    createInfo.pDepthStencilState = &depthStencilStateCreateInfo;
    createInfo.pColorBlendState = &colorBlendStateCreateInfo;
    createInfo.pDynamicState = &dynamicStateCreateInfo;
    createInfo.layout = *pipelineLayout_;

    pipeline_ = device_->graphicsPipeline(createInfo);

}

void CubeScene::createDescriptorPool() {
    descriptorPool_ =
        device_->descriptorPool()
            .flags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
            .addPoolSize({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1})
            .maxSets(1)
        .make_unique();
}

void CubeScene::createDescriptorSetLayout() {
    descriptorSetLayout_ =
        device_->descriptorSetLayout()
            .addBinding({ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT})
        .make_unique();
}

void CubeScene::updateDescriptorSet() {
    descriptorSet_ = descriptorPool_->allocate(*descriptorSetLayout_);
    VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    write.dstSet = descriptorSet_;
    write.dstBinding = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.descriptorCount = 1;

    VkDescriptorBufferInfo bufferInfo{*transformBuffer_, 0, VK_WHOLE_SIZE};
    write.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(*device_, 1, &write, 0, nullptr);

}

void CubeScene::invalidate() {
    initCamera();
    createDescriptorPool();
    createDescriptorSetLayout();
    updateDescriptorSet();
    createPipeline();
}

void CubeScene::initCamera() {
    transform[2] = vulkan_perspective(glm::radians(60.f), appState_.ScreenAspectRatio(), 0.1f, 50.f);
    transform[1] = glm::lookAt({0, 2, 5}, glm::vec3(0), {0, 1, 0});
    transform[0] = glm::mat4(1);

//    transform[2] = GL_TO_VKN_CLIP * transform[2];
}

void CubeScene::update() {
    static float angle = 0;
    static float speed = 100;
    angle = to<float>(Time::instance().delta()) * speed;
    transform[0] = glm::rotate(transform[0], glm::radians(angle), {0, 1, 0});
}

void CubeScene::record(VkCommandBuffer commandBuffer) {
    VkDeviceSize offset = 0;
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline_);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipelineLayout_, 0, 1, &descriptorSet_, 0, nullptr);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, *cubeVertices_, &offset);
    vkCmdBindIndexBuffer(commandBuffer, *cubeIndices_, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(commandBuffer, 36, 1, 0, 0, 0);
}

