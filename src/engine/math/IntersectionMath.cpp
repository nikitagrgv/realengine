#include "IntersectionMath.h"

#include "Bounds.h"
#include "Math.h"

void math::getDirectionTriangleIntersection(const glm::vec3 &origin, const glm::vec3 &direction,
    const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2,
    SimpleIntersection &out_intersection)
{
    const glm::vec3 dir_n = glm::normalize(direction);
    getDirectionTriangleIntersectionUnsafe(origin, dir_n, p0, p1, p2, out_intersection);
}

void math::getDirectionTriangleIntersectionUnsafe(const glm::vec3 &origin, const glm::vec3 &dir_n,
    const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2,
    SimpleIntersection &out_intersection)
{
    assert(math::isNormalized(dir_n));

    out_intersection.clear();

    constexpr float epsilon = std::numeric_limits<float>::epsilon();

    const glm::vec3 edge1 = p1 - p0;
    const glm::vec3 edge2 = p2 - p0;
    const glm::vec3 ray_cross_e2 = cross(dir_n, edge2);
    const float det = dot(edge1, ray_cross_e2);

    if (det > -epsilon && det < epsilon)
    {
        return; // This ray is parallel to this triangle.
    }

    const float inv_det = 1.0f / det;
    const glm::vec3 s = origin - p0;
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

    const glm::vec3 point = origin + dir_n * t;
    out_intersection.set(t, point);
}

void math::getDirectionBoundBoxIntersection(const glm::vec3 &origin, const glm::vec3 &direction,
    const BoundBox &bb, SimpleIntersection &out_intersection)
{
    const glm::vec3 dir_n = glm::normalize(direction);
    getDirectionBoundBoxIntersectionUnsafe(origin, dir_n, bb, out_intersection);
}

void math::getDirectionBoundBoxIntersectionUnsafe(const glm::vec3 &origin, const glm::vec3 &dir_n,
    const BoundBox &bb, SimpleIntersection &out_intersection)
{
    assert(math::isNormalized(dir_n));

    if (bb.contains(origin))
    {
        // TODO: FIX?
        out_intersection.set(0, origin);
        return;
    }

    const glm::vec3 lb = bb.getMin();
    const glm::vec3 rt = bb.getMax();

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
        out_intersection.clear();
        return;
    }

    // ray doesn't intersect AABB
    if (tmin > tmax)
    {
        out_intersection.clear();
        return;
    }

    const glm::vec3 point = origin + dir_n * tmin;
    out_intersection.set(tmin, point);
}
