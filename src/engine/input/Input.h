#pragma once

#include "InputCommon.h"
#include "events/Event.h"

#include <glm/vec2.hpp>
#include <vector>

class Input
{
public:
    Input();
    ~Input();

    bool isKeyDown(Key key) const;
    bool isKeyPressed(Key key) const;
    bool isKeyReleased(Key key) const;

    bool isButtonDown(Button button) const;
    bool isButtonPressed(Button button) const;
    bool isButtonReleased(Button button) const;

    glm::vec2 getMousePos() const;
    glm::vec2 getMouseDelta() const;

    int getWheel() const;
    int getHorizontalWheel() const;

    void setMouseGrabbed(bool grab);
    bool isMouseGrabbed() const;


    ////////////////////
    void update();
    void addEvent(EventPtr event);

private:
    std::vector<EventPtr> event_queue_;

    std::vector<bool> cur_pressed_keys_;
    std::vector<bool> old_pressed_keys_;

    std::vector<bool> cur_pressed_buttons_;
    std::vector<bool> old_pressed_buttons_;

    glm::vec2 mouse_pos_{};
    glm::vec2 mouse_delta_{};

    int vertical_wheel_{};
    int horizontal_wheel_{};
};
