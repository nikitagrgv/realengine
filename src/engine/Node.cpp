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
{}

Node::~Node() {}

void Node::setTransform(const glm::mat4 &transform)
{
    transform_ = transform;
    update_global_bound_box();
}

void Node::update_global_bound_box()
{
    global_bound_box_ = bound_box_.transformed(transform_);
}