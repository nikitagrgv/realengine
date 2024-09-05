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