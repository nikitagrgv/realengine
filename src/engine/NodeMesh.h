#pragma once

#include "Node.h"

class Mesh;

class NodeMesh : public Node
{
public:
    static Type getTypeStatic() { return Type::Mesh; }

    explicit NodeMesh(int id);

    Mesh *getMesh() const { return mesh_; }
    void setMesh(Mesh *mesh);

private:
    Mesh *mesh_{};
};
