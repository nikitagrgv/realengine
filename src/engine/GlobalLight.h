#pragma once

#include "glm/vec3.hpp"

struct GlobalLight
{
    glm::vec3 color{0.95, 0.89, 0.35};
    glm::vec3 dir;
};