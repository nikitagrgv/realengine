#pragma once
#include "signals/Signals.h"

class Gui
{
public:
    Gui();

    Signal<> &getSignalOnRender() { return on_render_; }

    void render();

    bool isWantCaptureMouse() const;

private:
    Signal<> on_render_;
};
