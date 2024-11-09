#pragma once

#include <glm/vec4.hpp>

struct FrustumPlanes
{
    union
    {
        glm::vec4 planes[6];
        struct
        {
            glm::vec4 left;
            glm::vec4 right;
            glm::vec4 bottom;
            glm::vec4 top;
            glm::vec4 near;
            glm::vec4 far;
        };
    };
};