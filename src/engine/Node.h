#pragma once

#include "Base.h"

#include <glm/mat4x4.hpp>
#include <string>

class Node
{
public:
    enum class Type
    {
        Mesh,
    };

public:
    REMOVE_COPY_MOVE_CLASS(Node);

    explicit Node(int id, Type type);
    virtual ~Node();

    Type getType() const { return type_; }

    int getId() const { return id_; }

    const std::string &getName() const { return name_; }
    void setName(std::string name) { name_ = std::move(name); }

    const glm::mat4 &getTransform() const { return transform_; }
    void setTransform(const glm::mat4 &transform) { transform_ = transform; }

private:
    const int id_{-1};
    const Type type_;
    std::string name_;
    glm::mat4 transform_{1.0f};
};
