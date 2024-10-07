#include "Node.h"

const char *Node::getTypeName(Type type)
{
    switch (type)
    {
    case Type::Mesh: return "Mesh";
    default: return "Unknown";
    }
}

Node::Node(int id, Type type)
    : id_(id)
    , type_(type)
{
    // TODO: NEEDED?
    // update_caches();
}

Node::~Node() {}

void Node::setTransform(const glm::mat4 &transform)
{
    transform_ = transform;
    update_caches();
}

void Node::getDirectionIntersection(const glm::vec3 &origin, const glm::vec3 &direction,
    SimpleIntersection &out_intersection)
{
    const glm::vec3 dir_n = glm::normalize(direction);
    getDirectionIntersectionUnsafe(origin, dir_n, out_intersection);
}

void Node::update_caches()
{
    itransform_ = glm::inverse(transform_);
    global_bound_box_ = bound_box_.transformed(transform_);
    // SEE CONSTRUCTOR!
}