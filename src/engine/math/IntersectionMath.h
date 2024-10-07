#pragma once

#include "Bounds.h"
#include "Intersection.h"

namespace math
{


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