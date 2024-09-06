#include "SystemProxy.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <iostream>


SystemProxy::SystemProxy()
{
    glfwInit();
}

SystemProxy::~SystemProxy()
{
    glfwTerminate();
}

GLFWwindow *SystemProxy::createWindow(int width, int height, const char *title)
{
    GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
        // eng.engine_->framebuffer_size_callback(window, width, height);
    });

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