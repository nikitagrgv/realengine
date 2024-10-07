#include "IntersectionMath.h"

#include "Bounds.h"
#include "Math.h"

void math::getDirectionTriangleIntersection(const glm::vec3 &origin, const glm::vec3 &direction,
    const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, SimpleIntersection &intersection)
{
    const glm::vec3 dir_n = glm::normalize(direction);
    getDirectionTriangleIntersectionUnsafe(origin, dir_n, a, b, c, intersection);
}

void math::getDirectionTriangleIntersectionUnsafe(const glm::vec3 &origin, const glm::vec3 &dir_n,
    const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c, SimpleIntersection &intersection)
{
    intersection.clear();

    constexpr float epsilon = std::numeric_limits<float>::epsilon();

    const glm::vec3 edge1 = b - a;
    const glm::vec3 edge2 = c - a;
    const glm::vec3 ray_cross_e2 = cross(dir_n, edge2);
    const float det = dot(edge1, ray_cross_e2);

    if (det > -epsilon && det < epsilon)
    {
        return; // This ray is parallel to this triangle.
    }

    const float inv_det = 1.0f / det;
    const glm::vec3 s = origin - a;
    const float u = inv_det * dot(s, ray_cross_e2);

    if (u < 0 || u > 1)
    {
        return;
    }

    const glm::vec3 s_cross_e1 = cross(s, edge1);
    const float v = inv_det * dot(dir_n, s_cross_e1);

    if (v < 0 || u + v > 1)
    {
        return;
    }

    // At this stage we can compute t to find out where the intersection point is on the line.
    const float t = inv_det * dot(edge2, s_cross_e1);

    if (t <= epsilon) // ray intersection
    {
        // This means that there is a line intersection but not a ray intersection.
        return;
    }
    intersection.valid = true;
    intersection.distance = t;
}

void math::getDirectionBoundBoxIntersection(const glm::vec3 &origin, const glm::vec3 &direction,
    const BoundBox &bb, SimpleIntersection &intersection)
{
    const glm::vec3 dir_n = glm::normalize(direction);
    getDirectionBoundBoxIntersectionUnsafe(origin, dir_n, bb, intersection);
}

void math::getDirectionBoundBoxIntersectionUnsafe(const glm::vec3 &origin, const glm::vec3 &dir_n,
    const BoundBox &bb, SimpleIntersection &intersection)
{
    const glm::vec3 lb = bb.getMin();
    const glm::vec3 rt = bb.getMax();

    assert(math::isEquals(math::length2(dir_n), 1));

    // TODO: FIX INTERSECTION INSIDE BB!

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
