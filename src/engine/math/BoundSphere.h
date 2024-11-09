#pragma once

#include "FrustumPlanes.h"

#include <glm/mat4x4.hpp>

namespace math
{

struct BoundSphere
{
public:
    BoundSphere() = default;

    explicit BoundSphere(const glm::vec4 &center_and_radius)
        : center_and_radius(center_and_radius)
    {}

    explicit BoundSphere(const glm::vec3 &center, float radius)
        : center_and_radius(center.x, center.y, center.z, radius)
    {}

    glm::vec3 getCenter() const { return glm::vec3(center_and_radius); }
    float getRadius() const { return center_and_radius.w; }

    bool isInsideFrustum(FrustumPlanes const &fru) const
    {
        for (const glm::vec4 &plane : fru.planes)
        {
            const float dist = glm::dot(getCenter(), glm::vec3(plane)) + plane.w + getRadius();
            if (dist < 0)
            {
                return false;
            }
        }
        return true;
    }

    void clear() { center_and_radius = glm::vec4{0.0f}; }

public:
    glm::vec4 center_and_radius{0.0f};
};

} // namespace math
