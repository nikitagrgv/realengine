#pragma once

#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/scalar_common.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/trigonometric.hpp"
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace math
{

inline float fastInvSqrt(float val)
{
    union Conv
    {
        float f;
        uint32_t i;
    };
    Conv conv{val};
    conv.i = 0x5f3759df - (conv.i >> 1);
    conv.f *= 1.5F - (val * 0.5F * conv.f * conv.f);
    return conv.f;
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

// Rotation: intrinsic XZY (extrinsic yzx)
// Angles in radians
inline void decompose(const glm::mat4 &m, glm::vec3 &pos, glm::vec3 &scale, glm::vec3 &angles)
{
    scale.x = glm::length(m[0]);
    scale.y = glm::length(m[1]);
    scale.z = glm::length(m[2]);

    {
        glm::vec3 inv_scale(1 / scale.x, 1 / scale.y, 1 / scale.z);

        const float v10 = m[1][0] * inv_scale.y;

        if (v10 < 0.99999f)
        {
            if (v10 > -0.99999f)
            {
                const float v00 = m[0][0] * inv_scale.x;
                const float v12 = m[1][2] * inv_scale.y;
                const float v11 = m[1][1] * inv_scale.y;
                const float v20 = m[2][0] * inv_scale.z;

                angles.z = std::asin(-v10);
                angles.x = std::atan2(v12, v11);
                angles.y = std::atan2(v20, v00);
            }
            else
            {
                const float v02 = m[0][2] * inv_scale.x;
                const float v22 = m[2][2] * inv_scale.z;

                angles.z = glm::half_pi<float>();
                angles.x = -std::atan2(-v02, v22);
                angles.y = 0;
            }
        }
        else
        {
            const float v02 = m[0][2] * inv_scale.x;
            const float v22 = m[2][2] * inv_scale.z;

            angles.z = -glm::half_pi<float>();
            angles.x = std::atan2(-v02, v22);
            angles.y = 0;
        }
    }
}

inline void decomposeDegrees(const glm::mat4 &m, glm::vec3 &pos, glm::vec3 &scale, glm::vec3 &angles)
{
    glm::vec3 angles_rad;
    decompose(m, pos, scale, angles_rad);
    angles = glm::degrees(angles_rad);
}

inline glm::mat4 compose(const glm::vec3 &pos, const glm::vec3 &scale, const glm::vec3 &angles)
{
    constexpr auto identity = glm::mat4{1.0f};

    const glm::vec3 s = glm::sin(angles);
    const glm::vec3 c = glm::cos(angles);

    glm::mat4 rot{1.0f};
    rot[0][0] = c.y * c.z;
    rot[1][0] = -s.z;
    rot[2][0] = c.z * s.y;

    rot[0][1] = s.x * s.y + c.x * c.y * s.z;
    rot[1][1] = c.x * c.z;
    rot[2][1] = -c.y * s.x + c.x * s.y * s.z;

    rot[0][2] = -c.x * s.y + c.y * s.x * s.z;
    rot[1][2] = c.z * s.x;
    rot[2][2] = c.x * c.y + s.x * s.y * s.z;

    constexpr float EPS = 0.001;
    constexpr float EPS2 = EPS * EPS;
    // TODO: remove out of here
    glm::vec3 corrected_scale = scale;
    if (corrected_scale.x * corrected_scale.x < EPS2)
    {
        corrected_scale.x = corrected_scale.x < 0 ? -EPS : EPS;
    }
    if (corrected_scale.y * corrected_scale.y < EPS2)
    {
        corrected_scale.y = corrected_scale.y < 0 ? -EPS : EPS;
    }
    if (corrected_scale.z * corrected_scale.z < EPS2)
    {
        corrected_scale.z = corrected_scale.z < 0 ? -EPS : EPS;
    }

    glm::mat4 v = glm::translate(identity, pos);
    // TODO: optimize
    v *= rot;
    v *= glm::scale(identity, corrected_scale);
    return v;
}

inline glm::mat4 composeDegrees(const glm::vec3 &pos, const glm::vec3 &scale, const glm::vec3 &angles)
{
    glm::vec3 angles_rad = glm::radians(angles);
    return compose(pos, scale, angles_rad);
}

} // namespace math
