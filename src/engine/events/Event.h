#pragma once

#include <cassert>
#include <memory>
#include <type_traits>

enum class EventType
{
    // input events
    INPUT_EVENTS_BEGIN,
    KeyPressEvent = INPUT_EVENTS_BEGIN,
    KeyReleaseEvent,
    ButtonPressEvent,
    ButtonReleaseEvent,
    MouseMoveEvent,
    MouseWheelEvent,
    INPUT_EVENTS_END,

    // window events,
    WINDOW_EVENTS_BEGIN,
    WindowResizeEvent = WINDOW_EVENTS_BEGIN,
    WindowCloseEvent,
    WINDOW_EVENTS_END,
};

// TODO: add timestamp/frame number?
class Event
{
public:
    virtual ~Event();
    [[nodiscard]] virtual EventType getType() const = 0; // TODO: make as class member
};

using EventPtr = std::unique_ptr<Event>;

template<typename To, typename From>
inline To *dyn_event_cast(From *p)
{
    return p->getType() == To::getTypeStatic() ? static_cast<To *>(p) : nullptr;
}

template<typename To, typename From>
inline To *static_event_cast(From *p)
{
    assert(p->getType() == To::getTypeStatic());
    return static_cast<To *>(p);
}
