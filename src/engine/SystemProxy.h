#pragma once

class Window;

class SystemProxy
{
public:
    SystemProxy();
    ~SystemProxy();

    Window *createWindow(int width, int height, const char *title);

private:
    bool glad_loaded_{false};
};
