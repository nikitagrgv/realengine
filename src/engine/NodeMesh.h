#pragma once

#include "Node.h"


class Material;
class Mesh;

class NodeMesh : public Node
{
public:
    static Type getTypeStatic() { return Type::Mesh; }

    explicit NodeMesh(int id);

    Mesh *getMesh() const { return mesh_; }
    void setMesh(Mesh *mesh);

    Material *getMaterial() const { return material_; }
    void setMaterial(Material *material) { material_ = material; }

private:
    Mesh *mesh_{};
    Material *material_{};
};
