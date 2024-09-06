#pragma once

struct GLFWwindow;

class SystemProxy
{
public:
    SystemProxy();
    ~SystemProxy();

    GLFWwindow *createWindow(int width, int height, const char *title);

private:
    bool glad_loaded_{false};
};
