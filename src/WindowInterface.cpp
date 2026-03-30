#include "Types.hpp"
#include "WindowInterface.hpp"
#include "event/Events.hpp"
#include <spdlog/spdlog.h>
#include <cstdlib>
#include <stdexcept>

std::vector<std::shared_ptr<Window>> WindowInterface::windows{};

void WindowInterface::connect() {
    glfwSetErrorCallback([](int error, const char* msg){ spdlog::error("Window Error: {}, message: {}", error, msg); });
    if(!glfwInit()){
        spdlog::error("unable to initialize window");
        std::exit(100);
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
}

void WindowInterface::disconnect() {
    for(const auto& window : windows) {
        glfwDestroyWindow(window->window_);
        window->connected_ = false;
    }
    glfwTerminate();
}

std::vector<const char *> WindowInterface::extensions() {
    uint32 count;
    auto exts = glfwGetRequiredInstanceExtensions(&count);
    if(exts == nullptr || count == 0) {
        spdlog::error("GLFW did not provide the required Vulkan instance extensions");
        throw std::runtime_error("GLFW did not provide the required Vulkan instance extensions");
    }

    std::vector<const char*> vec;

    for(auto i = 0; i < count; ++i){
        vec.push_back(exts[i]);
    }
    return vec;
}

std::shared_ptr<Window> WindowInterface::make_shared(int width, int height, std::string title) {
    auto window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if(window == nullptr) {
        spdlog::error("unable to create GLFW window");
        throw std::runtime_error("unable to create GLFW window");
    }
    glfwSetWindowCloseCallback(window, [](auto window){ glfwSetWindowShouldClose(window, GLFW_TRUE); });
    glfwSetFramebufferSizeCallback(window, [](auto window, int width, int height){
        EventBus::Publish(FrameBufferResizeEvent(width, height));
    });
    windows.push_back(std::make_shared<Window>(window, title));

    return windows.back();
}

void WindowInterface::pollEvents() {
    glfwPollEvents();
}


Window::Window(GLFWwindow* window, std::string title)
        :window_(window),
         title_(std::move(title)){
}

Window::~Window() {
    if(connected_) {
        glfwDestroyWindow(window_);
    }
}

bool Window::isActive() {
    return glfwWindowShouldClose(window_) != GLFW_TRUE;
}

VkSurfaceKHR Window::createSurface(VkInstance instance) {
    VkSurfaceKHR surface{};
    auto result = glfwCreateWindowSurface(instance, window_, VK_NULL_HANDLE, &surface);
    if(result != VK_SUCCESS) {
        spdlog::error("unable to create Vulkan surface from GLFW window, vk result={}", static_cast<int>(result));
        throw std::runtime_error("unable to create Vulkan surface from GLFW window");
    }
    return surface;
}

void Window::close() {
    glfwSetWindowShouldClose(window_, GLFW_TRUE);
}

void Window::aWaitEvents() {
    int width, height;
    do{
        glfwGetFramebufferSize(window_, &width, &height);
        glfwWaitEvents();
    }while(width == 0 && height == 0);
}

std::tuple<uint32, uint32> Window::dimensions() {
    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);
    return std::make_tuple(to<uint32>(width), to<uint32>(height));
}
