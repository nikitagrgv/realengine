#include "SystemProxy.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "Window.h"

#include <iostream>


SystemProxy::SystemProxy()
{
    glfwInit();
    glfwSetErrorCallback([](int error, const char *description) {
        std::cout << "GLFW Error: " << description << std::endl;
    });
}

SystemProxy::~SystemProxy()
{
    glfwTerminate();
}

Window *SystemProxy::createWindow(int width, int height, const char *title)
{
    Window *window = new Window(width, height, title);

    if (!glad_loaded_)
    {
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
        }
        glad_loaded_ = true;
    }

    return window;
}