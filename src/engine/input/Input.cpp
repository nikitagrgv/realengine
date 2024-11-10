#include "Input.h"

// clang-format off
#include <GLFW/glfw3.h>
// clang-format on

#include "EngineGlobals.h"
#include "Window.h"
#include "events/InputEvents.h"
#include "profiler/ScopedProfiler.h"
#include "utils/Algos.h"

#include <cassert>
#include <iostream>

bool Input::isKeyDown(Key key) const
{
    return keys_state_[(int)key];
}

bool Input::isKeyPressed(Key key) const
{
    return pressed_keys_[(int)key];
}

bool Input::isKeyReleased(Key key) const
{
    return released_keys_[(int)key];
}

bool Input::isButtonDown(Button button) const
{
    return buttons_state_[(int)button];
}

bool Input::isButtonPressed(Button button) const
{
    return pressed_buttons_[(int)button];
}

bool Input::isButtonReleased(Button button) const
{
    return released_buttons_[(int)button];
}

glm::vec2 Input::getMousePos() const
{
    return mouse_pos_;
}

glm::vec2 Input::getMouseDelta() const
{
    return mouse_delta_;
}

int Input::getWheel() const
{
    return vertical_wheel_;
}

int Input::getHorizontalWheel() const
{
    return horizontal_wheel_;
}

void Input::setMouseGrabbed(bool grab)
{
    eng.window->setMouseGrabbed(grab);
}

bool Input::isMouseGrabbed() const
{
    return eng.window->isMouseGrabbed();
}

Input::Input()
{
    keys_state_ = std::vector<bool>((int)Key::KEY_SIZE, false);
    pressed_keys_ = std::vector<bool>((int)Key::KEY_SIZE, false);
    released_keys_ = std::vector<bool>((int)Key::KEY_SIZE, false);

    buttons_state_ = std::vector<bool>((int)Button::BUTTON_SIZE, false);
    pressed_buttons_ = std::vector<bool>((int)Button::BUTTON_SIZE, false);
    released_buttons_ = std::vector<bool>((int)Button::BUTTON_SIZE, false);
}

Input::~Input() {}

void Input::update()
{
    SCOPED_PROFILER;

    const glm::vec2 new_pos = eng.window->getCursorPos();
    mouse_delta_ = new_pos - mouse_pos_;
    mouse_pos_ = new_pos;

    vertical_wheel_ = 0;
    horizontal_wheel_ = 0;

    Alg::fill(pressed_keys_, false);
    Alg::fill(released_keys_, false);
    Alg::fill(pressed_buttons_, false);
    Alg::fill(released_buttons_, false);

    // TODO: buffered events
    for (const EventPtr &event : event_queue_)
    {
        switch (event->getType())
        {
        case EventType::KeyPressEvent:
        {
            auto *e = static_event_cast<KeyPressEvent>(event.get());
            keys_state_[(int)e->key_] = true;
            pressed_keys_[(int)e->key_] = true;
            break;
        }
        case EventType::KeyReleaseEvent:
        {
            auto *e = static_event_cast<KeyReleaseEvent>(event.get());
            keys_state_[(int)e->key_] = false;
            released_keys_[(int)e->key_] = true;
            break;
        }
        case EventType::ButtonPressEvent:
        {
            auto *e = static_event_cast<ButtonPressEvent>(event.get());
            buttons_state_[(int)e->button_] = true;
            pressed_buttons_[(int)e->button_] = true;
            break;
        }
        case EventType::ButtonReleaseEvent:
        {
            auto *e = static_event_cast<ButtonReleaseEvent>(event.get());
            buttons_state_[(int)e->button_] = false;
            released_buttons_[(int)e->button_] = true;
            break;
        }
        case EventType::MouseWheelEvent:
        {
            auto *e = static_event_cast<MouseWheelEvent>(event.get());
            vertical_wheel_ += e->ver_;
            horizontal_wheel_ += e->hor_;
            break;
        }

        default: assert(0 && "Unhandled event");
        }
    }
    event_queue_.clear();
}

void Input::addEvent(EventPtr event)
{
    assert(event);
    assert(event->getType() >= EventType::INPUT_EVENTS_BEGIN
        && event->getType() < EventType::INPUT_EVENTS_END);

    event_queue_.push_back(std::move(event));
}
