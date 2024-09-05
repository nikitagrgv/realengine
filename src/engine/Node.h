#pragma once

#include <glm/mat4x4.hpp>
#include <string>

class Node
{
public:
    virtual ~Node();

    const std::string &getName() const { return name_; }
    void setName(std::string name) { name_ = std::move(name); }

    const glm::mat4 &getTransform() const { return transform_; }
    void setTransform(const glm::mat4 &transform) { transform_ = transform; }

private:
    std::string name_;
    glm::mat4 transform_{1.0f};
};
