#pragma once

#include "Bounds.h"
#include "Intersection.h"

namespace math
{

inline void getDirectionIntersection(const BoundBox &bb, const glm::vec3 &origin,
    const glm::vec3 &direction, SimpleIntersection &intersection)
{
    const glm::vec3 lb = bb.getMin();
    const glm::vec3 rt = bb.getMax();
    const glm::vec3 dir_n = glm::normalize(direction);

    glm::vec3 dirfrac;
    dirfrac.x = 1.0f / dir_n.x;
    dirfrac.y = 1.0f / dir_n.y;
    dirfrac.z = 1.0f / dir_n.z;

    float t1 = (lb.x - origin.x) * dirfrac.x;
    float t2 = (rt.x - origin.x) * dirfrac.x;
    float t3 = (lb.y - origin.y) * dirfrac.y;
    float t4 = (rt.y - origin.y) * dirfrac.y;
    float t5 = (lb.z - origin.z) * dirfrac.z;
    float t6 = (rt.z - origin.z) * dirfrac.z;

    using glm::min;
    using glm::max;
    float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
    float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

    // line is intersecting AABB, but the whole AABB is behind us
    if (tmax < 0)
    {
        intersection.clear();
        return;
    }

    // ray doesn't intersect AABB
    if (tmin > tmax)
    {
        intersection.clear();
        return;
    }

    intersection.distance = tmin;
    intersection.valid = true;
}


} // namespace math