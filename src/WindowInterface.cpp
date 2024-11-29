#include "WindowInterface.hpp"

#include <spdlog/spdlog.h>
#include <cstdlib>

std::vector<std::shared_ptr<Window>> WindowInterface::windows{};

void WindowInterface::connect() {
    glfwSetErrorCallback([](int error, const char* msg){ spdlog::error("Window Error: {}, message: {}", error, msg); });
    if(!glfwInit()){
        spdlog::error("unable to initialize window");
        std::exit(100);
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
}

void WindowInterface::disconnect() {
    for(const auto& window : windows) {
        glfwDestroyWindow(window->_window);
        window->_connected = false;
    }
    glfwTerminate();
}

std::vector<const char *> WindowInterface::extensions() {
    uint32_t count;
    auto exts = glfwGetRequiredInstanceExtensions(&count);

    std::vector<const char*> vec;

    for(auto i = 0; i < count; ++i){
        vec.push_back(exts[i]);
    }
    return vec;
}

std::shared_ptr<Window> WindowInterface::make_shared(int width, int height, std::string title) {
    auto window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    glfwSetWindowCloseCallback(window, [](auto window){ glfwSetWindowShouldClose(window, GLFW_TRUE); });
    windows.push_back(std::make_shared<Window>(window, title));

    return windows.back();
}

void WindowInterface::pollEvents() {
    glfwPollEvents();
}


Window::Window(GLFWwindow* window, std::string title)
        :_window(window),
         _title(std::move(title)){
}

Window::~Window() {
    if(_connected) {
        glfwDestroyWindow(_window);
    }
}

bool Window::isActive() {
    return glfwWindowShouldClose(_window) != GLFW_TRUE;
}

VkSurfaceKHR Window::createSurface(VkInstance instance) {
    VkSurfaceKHR surface{};
    glfwCreateWindowSurface(instance, _window, VK_NULL_HANDLE, &surface);
    return surface;
}

void Window::close() {
    glfwSetWindowShouldClose(_window, GLFW_TRUE);
}

void Window::aWaitEvents() {
    int width, height;
    do{
        glfwGetFramebufferSize(_window, &width, &height);
        glfwWaitEvents();
    }while(width == 0 && height == 0);
}

std::tuple<uint32_t, uint32_t> Window::dimensions() {
    int width, height;
    glfwGetFramebufferSize(_window, &width, &height);
    return std::make_tuple(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
}
