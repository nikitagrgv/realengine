#pragma once

#include "Common.h"
#include "events/Event.h"

// TODO: setters+getters instead of public fields

class WindowResizeEvent : public Event
{
public:
    WindowResizeEvent(WinHandle window, int width, int height)
        : window_(window)
        , width_(width)
        , height_(height)
    {}

    [[nodiscard]] EventType getType() const override { return getTypeStatic(); }
    static EventType getTypeStatic() { return EventType::WindowResizeEvent; }

    WinHandle window_;
    int width_{};
    int height_{};
};

class WindowCloseEvent : public Event
{
public:
    WindowCloseEvent(WinHandle window)
        : window_(window)
    {}

    [[nodiscard]] EventType getType() const override { return getTypeStatic(); }
    static EventType getTypeStatic() { return EventType::WindowCloseEvent; }

    WinHandle window_;
};
