#pragma once

#include <glm/vec3.hpp>

struct Ray
{
    Ray() = default;
    Ray(const glm::vec3 &begin, const glm::vec3 &end)
        : begin(begin)
        , end(end)
    {}

    glm::vec3 begin{};
    glm::vec3 end{};
};
