#pragma once

#include "events/Event.h"
#include "input/InputCommon.h"

// TODO: setters+getters instead of public fields

class KeyPressEvent : public Event
{
public:
    explicit KeyPressEvent(Key key)
        : key_(key)
    {}

    EventType getType() const override { return getTypeStatic(); }
    static EventType getTypeStatic() { return EventType::KeyPressEvent; }

    Key key_;
};

class KeyReleaseEvent : public Event
{
public:
    explicit KeyReleaseEvent(Key key)
        : key_(key)
    {}

    EventType getType() const override { return getTypeStatic(); }
    static EventType getTypeStatic() { return EventType::KeyReleaseEvent; }

public:
    Key key_;
};

class ButtonPressEvent : public Event
{
public:
    ButtonPressEvent(Button button, float x, float y)
        : button_(button)
        , x_(x)
        , y_(y)
    {}

    EventType getType() const override { return getTypeStatic(); }
    static EventType getTypeStatic() { return EventType::ButtonPressEvent; }

    Button button_;
    float x_{};
    float y_{};
};

class ButtonReleaseEvent : public Event
{
public:
    ButtonReleaseEvent(Button button, float x, float y)
        : button_(button)
        , x_(x)
        , y_(y)
    {}

    EventType getType() const override { return getTypeStatic(); }
    static EventType getTypeStatic() { return EventType::ButtonReleaseEvent; }

public:
    Button button_;
    float x_{};
    float y_{};
};

class MouseMoveEvent : public Event
{
public:
    MouseMoveEvent(float dx, float dy, float x, float y)
        : dx_(dx)
        , dy_(dy)
        , x_(x)
        , y_(y)
    {}

    EventType getType() const override { return getTypeStatic(); }
    static EventType getTypeStatic() { return EventType::MouseMoveEvent; }

public:
    float dx_{};
    float dy_{};
    float x_{};
    float y_{};
};

class MouseWheelEvent : public Event
{
public:
    MouseWheelEvent(int ver, int hor)
        : ver_(ver)
        , hor_(hor)
    {}

    EventType getType() const override { return getTypeStatic(); }
    static EventType getTypeStatic() { return EventType::MouseWheelEvent; }

public:
    int ver_{};
    int hor_{};
};