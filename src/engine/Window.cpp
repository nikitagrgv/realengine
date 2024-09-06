#include "Window.h"

// clang-format off
#include <GLFW/glfw3.h>
// clang-format on

#include <cassert>
#include <iostream>

Window *Window::INSTANCE = nullptr;

Window::Window(int width, int height, const char *title)
{
    assert(!INSTANCE);
    INSTANCE = this;


    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // TODO add flag

    window_ = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window_ == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
    }
    bind();

    glfwSetFramebufferSizeCallback(window_, &Window::framebuffer_size_callback);
    glfwSetKeyCallback(window_, key_callback);
    glfwSetMouseButtonCallback(window_, button_callback);
    glfwSetCursorPosCallback(window_, cursor_move_callback);
    glfwSetScrollCallback(window_, scroll_callback);
    glfwSetWindowCloseCallback(window_, close_callback);
}

Window::~Window()
{
    assert(INSTANCE == this);
    INSTANCE = nullptr;
}

void Window::maximize()
{
    glfwMaximizeWindow(window_);
}

int Window::getWidth()
{
    return getSize().x;
}

int Window::getHeight()
{
    return getSize().y;
}

glm::ivec2 Window::getSize()
{
    glm::ivec2 v;
    glfwGetWindowSize(window_, &v.x, &v.y);
    return v;
}

float Window::getCursorX() const
{
    return getCursorPos().x;
}

float Window::getCursorY() const
{
    return getCursorPos().y;
}

glm::vec2 Window::getCursorPos() const
{
    double x, y;
    glfwGetCursorPos(window_, &x, &y);
    return glm::vec2(x, y);
}

void Window::setMouseGrabbed(bool grab)
{
    const int mode = grab ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
    glfwSetInputMode(window_, GLFW_CURSOR, mode);
}

bool Window::isMouseGrabbed() const
{
    const int mode = glfwGetInputMode(window_, GLFW_CURSOR);
    return mode == GLFW_CURSOR_DISABLED;
}

void Window::bind()
{
    glfwMakeContextCurrent(window_);
}

void Window::swap()
{
    glfwSwapBuffers(window_);
}

GLFWwindow *Window::getHandle()
{
    return window_;
}

void Window::framebuffer_size_callback(int width, int height)
{
    signal_resized_(width, height);
}

void Window::key_callback(int key, int scancode, int action, int mods) {}

void Window::button_callback(int button, int action, int mods) {}

void Window::cursor_move_callback(double xpos, double ypos) {}

void Window::scroll_callback(double xoffset, double yoffset) {}

void Window::close_callback()
{
    signal_close_requested_.call();
}

void Window::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    assert(INSTANCE->window_ == window);
    INSTANCE->framebuffer_size_callback(width, height);
}

void Window::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    assert(INSTANCE->window_ == window);
    INSTANCE->key_callback(key, scancode, action, mods);
}

void Window::button_callback(GLFWwindow *window, int button, int action, int mods)
{
    assert(INSTANCE->window_ == window);
    INSTANCE->button_callback(button, action, mods);
}

void Window::cursor_move_callback(GLFWwindow *window, double xpos, double ypos)
{
    assert(INSTANCE->window_ == window);
    INSTANCE->cursor_move_callback(xpos, ypos);
}

void Window::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    assert(INSTANCE->window_ == window);
    INSTANCE->scroll_callback(xoffset, yoffset);
}

void Window::close_callback(GLFWwindow *window)
{
    assert(INSTANCE->window_ == window);
    INSTANCE->close_callback();
}
