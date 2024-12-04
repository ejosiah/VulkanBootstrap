#pragma once
#include <volk.h>
#include <GLFW/glfw3.h>

#include <memory>
#include <vector>
#include <string>
#include <tuple>

class Window;

class WindowInterface {
public:
    static void connect();

    static void disconnect();

    static std::vector<const char*> extensions();

    static std::shared_ptr<Window> make_shared(int width, int height, std::string title);

    static void pollEvents();

private:
    WindowInterface() = default;
    static std::vector<std::shared_ptr<Window>> windows;
};

class Window{
public:
    friend class WindowInterface;
    Window(GLFWwindow* window, std::string title);

    ~Window();

    bool isActive();

    void close();

    void aWaitEvents();

    std::tuple<uint32_t, uint32_t> dimensions();

    VkSurfaceKHR createSurface(VkInstance instance);

    operator GLFWwindow*() const {
        return window_;
    }

private:
    std::string title_;
    GLFWwindow* window_{};
    bool connected_{true};
};