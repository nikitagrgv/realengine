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

void Camera::update_viewproj()
{
    viewproj_ = proj_ * view_;
}