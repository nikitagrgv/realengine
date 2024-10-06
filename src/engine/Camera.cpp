#include "Camera.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/trigonometric.hpp"

Camera::Camera()
    : proj_(glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f))
{}

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
}