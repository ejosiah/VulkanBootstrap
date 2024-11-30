#include "Types.hpp"
#include "Time.hpp"
#include "event/Events.hpp"
#include "Scene.hpp"

#include <utility>

Scene::Scene(std::shared_ptr<VulkanDevice> device)
: _device(std::move(device)){}

void Scene::init0() {
    _commandPool = _device->createCommandPool(VK_QUEUE_GRAPHICS_BIT);
    init();
}


TestScene::TestScene(std::shared_ptr<VulkanDevice> _device)
: Scene(std::move(_device)) {}

void TestScene::update() {
    static int period = 5;
    const auto elapsed = Time::Elapsed();

    if(int(elapsed) % period == 0){
        int index = int(elapsed)/period % to<int>(_clearColors.size());
        auto cc = _clearColors[index].float32;
        Events::ClearScreen(cc[0], cc[1], cc[2], cc[3]);
    }
}

