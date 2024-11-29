#pragma once

#include <volk.h>

class Scene {
public:
    Scene() = default;
    ~Scene() = default;

    virtual void init() {}

    virtual void record(VkCommandBuffer commandBuffer) = 0;

    virtual void update(float time) = 0;
};