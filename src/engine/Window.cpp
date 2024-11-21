#include "Window.h"

// clang-format off
#include <GLFW/glfw3.h>
// clang-format on

#include "EngineGlobals.h"
#include "events/Event.h"
#include "events/InputEvents.h"
#include "input/Input.h"
#include "input/InputUtils.h"
#include "profiler/ScopedProfiler.h"

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
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // TODO add flag
    // glfwWindowHint(GLFW_SAMPLES, 4);

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

int Window::getWidth() const
{
    return getSize().x;
}

int Window::getHeight() const
{
    return getSize().y;
}

glm::ivec2 Window::getSize() const
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

glm::vec2 Window::getNormalizedCursorPos() const
{
    return getCursorPos() / glm::vec2(getSize());
}

glm::vec2 Window::mapToNormalized(glm::vec2 pixel_pos) const
{
    return pixel_pos / glm::vec2(getSize());
}

glm::vec2 Window::mapToNormalized(glm::ivec2 pixel_pos) const
{
    return mapToNormalized(glm::vec2(pixel_pos));
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
    SCOPED_FUNC_PROFILER;
    glfwMakeContextCurrent(window_);
}

void Window::swap()
{
    SCOPED_FUNC_PROFILER;
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

void Window::key_callback(int key, int scancode, int action, int mods)
{
    Key k = utils::keyFromGlfw(key);
    if (k == Key::KEY_UNKNOWN)
    {
        return;
    }

    if (action == GLFW_PRESS)
    {
        EventPtr event = std::make_unique<KeyPressEvent>(k);
        eng.input->addEvent(std::move(event));
    }
    if (action == GLFW_RELEASE)
    {
        EventPtr event = std::make_unique<KeyReleaseEvent>(k);
        eng.input->addEvent(std::move(event));
    }
}

void Window::button_callback(int button, int action, int mods)
{
    Button b = utils::buttonFromGlfw(button);
    glm::ivec2 p = eng.input->getMousePos();

    if (action == GLFW_PRESS)
    {
        EventPtr event = std::make_unique<ButtonPressEvent>(b, (float)p.x, (float)p.y);
        eng.input->addEvent(std::move(event));
    }
    if (action == GLFW_RELEASE)
    {
        EventPtr event = std::make_unique<ButtonReleaseEvent>(b, (float)p.x, (float)p.y);
        eng.input->addEvent(std::move(event));
    }
}

void Window::cursor_move_callback(double xpos, double ypos)
{
    // Input handles this by itself
}

void Window::scroll_callback(double xoffset, double yoffset)
{
    EventPtr event = std::make_unique<MouseWheelEvent>((int)yoffset, (int)xoffset);
    eng.input->addEvent(std::move(event));
}

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
