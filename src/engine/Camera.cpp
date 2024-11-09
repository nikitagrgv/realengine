#include "Camera.h"

#include "math/Math.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/trigonometric.hpp"

Camera::Camera()
{
    setPerspective(45.0f, 1.0f, 0.1f, 100.0f);
}

Camera::Camera(const glm::mat4 &proj)
    : proj_(proj)
{
    update_cached();
}

void Camera::setPerspective(float fov_deg, float aspect, float z_near, float z_far)
{
    setProj(glm::perspective(glm::radians(fov_deg), aspect, z_near, z_far));
}

Ray Camera::getNearFarRay(glm::vec2 normalized_screen_pos) const
{
    glm::vec2 gl_pos;
    gl_pos.x = normalized_screen_pos.x * (2.0f) - 1.0f;
    gl_pos.y = normalized_screen_pos.y * (-2.0f) + 1.0f;

    glm::vec4 near = iviewproj_ * glm::vec4{gl_pos.x, gl_pos.y, -1, 1};
    near /= near.w;

    glm::vec4 far = iviewproj_ * glm::vec4{gl_pos.x, gl_pos.y, 1, 1};
    far /= far.w;

    return Ray{near, far};
}

void Camera::update_cached()
{
    viewproj_ = proj_ * view_;
    iviewproj_ = glm::inverse(viewproj_);

    // clang-format off
    // https://stackoverflow.com/a/34960913/19031745
    // TODO: shitty?
    for (int i = 4; i--; ) { planes_.left[i]   = viewproj_[i][3] + viewproj_[i][0]; }
    for (int i = 4; i--; ) { planes_.right[i]  = viewproj_[i][3] - viewproj_[i][0]; }
    for (int i = 4; i--; ) { planes_.bottom[i] = viewproj_[i][3] + viewproj_[i][1]; }
    for (int i = 4; i--; ) { planes_.top[i]    = viewproj_[i][3] - viewproj_[i][1]; }
    for (int i = 4; i--; ) { planes_.near[i]   = viewproj_[i][3] + viewproj_[i][2]; }
    for (int i = 4; i--; ) { planes_.far[i]    = viewproj_[i][3] - viewproj_[i][2]; }
    // clang-format on

    for (glm::vec4 &p : planes_.planes)
    {
        const float length = glm::length(glm::vec3(p));
        p /= length;
    }

#ifndef NDEBUG
    for (auto p : planes_.planes)
    {
        assert(math::isNormalized(glm::vec3(p)));
    }
#endif
}
