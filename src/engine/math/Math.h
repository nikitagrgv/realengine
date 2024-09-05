#pragma once

#include "glm/ext/scalar_common.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace math
{

inline float fastInvSqrt(float val)
{
    static_assert(std::numeric_limits<float>::is_iec559);
    const float y = std::bit_cast<float>(0x5f3759df - (std::bit_cast<std::uint32_t>(val) >> 1));
    return y * (1.5f - (val * 0.5f * y * y));
}

inline float length2(glm::vec2 v)
{
    return v.x * v.x + v.y * v.y;
}

inline float length2(glm::vec3 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

inline float length2(const glm::vec4 &v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

inline float fastInvLength(glm::vec2 v)
{
    return fastInvSqrt(length2(v));
}

inline float fastInvLength(glm::vec3 v)
{
    return fastInvSqrt(length2(v));
}

inline float fastInvLength(const glm::vec4 &v)
{
    return fastInvSqrt(length2(v));
}

inline glm::vec3 maxByComponens(const glm::vec3 &a, const glm::vec3 &b)
{
    glm::vec3 ret;
    ret.x = glm::max(a.x, b.x);
    ret.y = glm::max(a.y, b.y);
    ret.z = glm::max(a.z, b.z);
    return ret;
}

inline glm::vec3 minByComponens(const glm::vec3 &a, const glm::vec3 &b)
{
    glm::vec3 ret;
    ret.x = glm::min(a.x, b.x);
    ret.y = glm::min(a.y, b.y);
    ret.z = glm::min(a.z, b.z);
    return ret;
}

} // namespace math
