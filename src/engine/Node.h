#pragma once

#include "Base.h"
#include "math/BoundBox.h"

#include <glm/mat4x4.hpp>
#include <string>

struct SimpleIntersection;

class Node
{
public:
    enum class Type
    {
        Mesh,
    };

public:
    static const char *getTypeName(Type type);

    REMOVE_COPY_MOVE_CLASS(Node);

    explicit Node(int id, Type type);
    virtual ~Node();

    template<typename T>
    T *cast();

    template<typename T>
    const T *cast() const;

    Type getType() const { return type_; }
    const char *getTypeName() const { return getTypeName(type_); }

    int getId() const { return id_; }

    const std::string &getName() const { return name_; }
    void setName(std::string name) { name_ = std::move(name); }

    const glm::mat4 &getTransform() const { return transform_; }
    void setTransform(const glm::mat4 &transform);

    const glm::mat4 &getITransform() const { return itransform_; }

    const math::BoundBox &getBoundBox() const { return bound_box_; }
    const math::BoundBox &getGlobalBoundBox() const { return global_bound_box_; }

    bool isEnabled() const { return enabled_; }
    void setEnabled(bool enabled) { enabled_ = enabled; }

    virtual void getDirectionIntersection(const glm::vec3 &origin, const glm::vec3 &direction,
        SimpleIntersection &out_intersection);

    // Direction must be normalized
    virtual void getDirectionIntersectionUnsafe(const glm::vec3 &origin, const glm::vec3 &dir_n,
        SimpleIntersection &out_intersection)
        = 0;

protected:
    void update_caches();

protected:
    math::BoundBox bound_box_;

private:
    bool enabled_{true};
    const int id_{-1};
    const Type type_;
    std::string name_;
    glm::mat4 transform_{1.0f};
    glm::mat4 itransform_{1.0f};

    math::BoundBox global_bound_box_;
};

template<typename T>
T *Node::cast()
{
    if (getType() == T::getTypeStatic())
    {
        assert(dynamic_cast<T *>(this));
        return static_cast<T *>(this);
    }
    return nullptr;
}


template<typename T>
const T *Node::cast() const
{
    if (getType() == T::getTypeStatic())
    {
        assert(dynamic_cast<const T *>(this));
        return static_cast<const T *>(this);
    }
    return nullptr;
}