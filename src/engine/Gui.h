#pragma once
#include "signals/Signals.h"

class Gui
{
public:
    Gui();
    ~Gui();

    Signal<> &getSignalOnUpdate() { return on_update_; }

    void update();
    void swap();

private:
    Signal<> on_update_;
};
