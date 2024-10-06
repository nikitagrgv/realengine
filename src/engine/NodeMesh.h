#pragma once

#include "Node.h"


class Material;
class Mesh;

class NodeMesh final : public Node
{
public:
    static Type getTypeStatic() { return Type::Mesh; }

    explicit NodeMesh(int id);

    const Mesh *getMesh() const { return mesh_; }
    Mesh *takeMesh();
    void setMesh(Mesh *mesh);

    Material *getMaterial() const { return material_; }
    void setMaterial(Material *material) { material_ = material; }

private:
    void update_bounds();

private:
    Mesh *mesh_{};
    Material *material_{};
};
