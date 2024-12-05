#pragma once

#include "VulkanApplication.hpp"

struct BootStrap {
    using SceneFactory = std::function<std::shared_ptr<Scene>(std::shared_ptr<VulkanDevice>, const AppState&)>;

    static VulkanApplication application(SceneFactory &&sceneFactory);
};