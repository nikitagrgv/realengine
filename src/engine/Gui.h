#pragma once
#include "signals/Signals.h"

class Gui
{
public:
    Gui();

    Signal<> &getSignalOnRender() { return on_render_; }

    void render();

private:
    Signal<> on_render_;
};
