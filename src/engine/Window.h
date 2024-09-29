#pragma once

#include "signals/Signals.h"

#include <glm/vec2.hpp>

struct GLFWwindow;

class Window
{
public:
    Window(int width, int height, const char *title);
    ~Window();

    void maximize();

    int getWidth() const;
    int getHeight() const;
    glm::ivec2 getSize() const;

    float getCursorX() const;
    float getCursorY() const;

    glm::vec2 getCursorPos() const;

    glm::vec2 getNormalizedCursorPos() const;

    glm::vec2 mapToNormalized(glm::vec2 pixel_pos) const;
    glm::vec2 mapToNormalized(glm::ivec2 pixel_pos) const;

    void setMouseGrabbed(bool grab);
    bool isMouseGrabbed() const;

    void bind();
    void swap();

    GLFWwindow *getHandle();

    Signal<int, int> &getSignalResized() { return signal_resized_; }
    Signal<> &getSignalCloseRequested() { return signal_close_requested_; }

private:
    void framebuffer_size_callback(int width, int height);
    void key_callback(int key, int scancode, int action, int mods);
    void button_callback(int button, int action, int mods);
    void cursor_move_callback(double xpos, double ypos);
    void scroll_callback(double xoffset, double yoffset);
    void close_callback();

    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
    static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void button_callback(GLFWwindow *window, int button, int action, int mods);
    static void cursor_move_callback(GLFWwindow *window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
    static void close_callback(GLFWwindow *window);

private:
    GLFWwindow *window_{};

    Signal<int, int> signal_resized_;
    Signal<> signal_close_requested_;

    static Window *INSTANCE;
};
