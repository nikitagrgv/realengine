#include "Input.h"

// clang-format off
#include <GLFW/glfw3.h>
// clang-format on

#include "EngineGlobals.h"
#include "Window.h"
#include "events/InputEvents.h"

#include <cassert>
#include <iostream>

bool Input::isKeyDown(Key key) const
{
    return cur_pressed_keys_[(int)key];
}

bool Input::isKeyPressed(Key key) const
{
    return cur_pressed_keys_[(int)key] && !old_pressed_keys_[(int)key];
}

bool Input::isKeyReleased(Key key) const
{
    return !cur_pressed_keys_[(int)key] && old_pressed_keys_[(int)key];
}

bool Input::isButtonDown(Button button) const
{
    return cur_pressed_buttons_[(int)button];
}

bool Input::isButtonPressed(Button button) const
{
    return cur_pressed_buttons_[(int)button] && !old_pressed_buttons_[(int)button];
}

bool Input::isButtonReleased(Button button) const
{
    return !cur_pressed_buttons_[(int)button] && old_pressed_buttons_[(int)button];
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
    cur_pressed_keys_ = std::vector<bool>((int)Key::KEY_SIZE, false);
    old_pressed_keys_ = std::vector<bool>((int)Key::KEY_SIZE, false);

    cur_pressed_buttons_ = std::vector<bool>((int)Button::BUTTON_SIZE, false);
    old_pressed_buttons_ = std::vector<bool>((int)Button::BUTTON_SIZE, false);
}

Input::~Input() {}

void Input::update()
{
    const glm::vec2 new_pos = eng.window->getCursorPos();
    mouse_delta_ = new_pos - mouse_pos_;
    mouse_pos_ = new_pos;

    vertical_wheel_ = 0;
    horizontal_wheel_ = 0;

    old_pressed_keys_ = cur_pressed_keys_;
    old_pressed_buttons_ = cur_pressed_buttons_;

    // TODO: buffered events
    for (const EventPtr &event : event_queue_)
    {
        switch (event->getType())
        {
        case EventType::KeyPressEvent:
        {
            auto *e = static_event_cast<KeyPressEvent>(event.get());
            cur_pressed_keys_[(int)e->key_] = true;
            break;
        }
        case EventType::KeyReleaseEvent:
        {
            auto *e = static_event_cast<KeyReleaseEvent>(event.get());
            cur_pressed_keys_[(int)e->key_] = false;
            break;
        }
        case EventType::ButtonPressEvent:
        {
            auto *e = static_event_cast<ButtonPressEvent>(event.get());
            cur_pressed_buttons_[(int)e->button_] = true;
            break;
        }
        case EventType::ButtonReleaseEvent:
        {
            auto *e = static_event_cast<ButtonReleaseEvent>(event.get());
            cur_pressed_buttons_[(int)e->button_] = false;
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
