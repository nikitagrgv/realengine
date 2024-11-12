#pragma once

#include "Base.h"
#include "BoundBox.h"
#include "FrustumPlanes.h"
#include "Intersection.h"

namespace math
{

REALENGINE_INLINE bool getDirectionPlaneIntersectionDistance(const glm::vec3 &origin,
    const glm::vec3 &dir_n, const glm::vec3 &plane_n, float plane_d, float &distance)
{
    const float denom = glm::dot(plane_n, dir_n);
    if (glm::abs(denom) < 1e-9f)
    {
        distance = 1e9f;
        return false;
    }
    const float t = -(glm::dot(plane_n, origin) + plane_d) / denom;
    distance = t;
    if (t < 0.0f)
    {
        return false;
    }
    return true;
}

// Direction must be normalized
void getDirectionPlaneIntersectionUnsafe(const glm::vec3 &origin, const glm::vec3 &dir_n,
    const glm::vec4 &plane, SimpleIntersection &out_intersection);

void getDirectionTriangleIntersection(const glm::vec3 &origin, const glm::vec3 &direction,
    const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2,
    SimpleIntersection &out_intersection);
// Direction must be normalized
void getDirectionTriangleIntersectionUnsafe(const glm::vec3 &origin, const glm::vec3 &dir_n,
    const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2,
    SimpleIntersection &out_intersection);


void getDirectionBoundBoxIntersection(const glm::vec3 &origin, const glm::vec3 &direction,
    const BoundBox &bb, SimpleIntersection &out_intersection);
// Direction must be normalized
void getDirectionBoundBoxIntersectionUnsafe(const glm::vec3 &origin, const glm::vec3 &dir_n,
    const BoundBox &bb, SimpleIntersection &out_intersection);


} // namespace math