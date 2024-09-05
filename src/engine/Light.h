#pragma once

#include "glm/vec3.hpp"

class Light
{
public:
    glm::vec3 pos{0.0f};

    glm::vec3 color{1.0f, 1.0f, 1.0f};
    float ambient_power = 0.1f;
    float diffuse_power = 1.0f;
    float specular_power = 1.0f;

private:
};
