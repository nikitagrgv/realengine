#pragma once

#include "Ray.h"

#include "glm/mat4x4.hpp"

class Camera
{
public:
    Camera();

    Camera(const glm::mat4 &view, const glm::mat4 &proj)
        : view_(view)
        , proj_(proj)
    {
        update_cached();
    }

    explicit Camera(const glm::mat4 &proj)
        : proj_(proj)
    {
        update_cached();
    }

    glm::mat4 getMVP(const glm::mat4 &model) const { return viewproj_ * model; }

    void setPerspective(float fov_deg, float aspect, float z_near, float z_far);

    const glm::mat4 &getProj() const { return proj_; }
    void setProj(const glm::mat4 &proj)
    {
        proj_ = proj;
        update_cached();
    }

    const glm::mat4 &getView() const { return view_; }
    void setView(const glm::mat4 &view)
    {
        view_ = view;
        transform_ = glm::inverse(view_);
        update_cached();
    }

    const glm::mat4 &getTransform() const { return transform_; }
    void setTransform(const glm::mat4 &transform)
    {
        transform_ = transform;
        view_ = glm::inverse(transform_);
        update_cached();
    }

    glm::vec3 getPosition() const { return transform_[3]; }

    const glm::mat4 &getViewProj() const { return viewproj_; }

    Ray getNearFarRay(glm::vec2 normalized_mouse_pos) const;

private:
    void update_cached();

private:
    glm::mat4 transform_{1.0f};
    glm::mat4 view_{1.0f};
    glm::mat4 proj_{1.0f};
    glm::mat4 viewproj_{1.0f};
    glm::mat4 iviewproj_{1.0f};
};