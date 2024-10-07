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

    void getDirectionIntersection(const glm::vec3 &origin, const glm::vec3 &direction,
        SimpleIntersection &out_intersection) override;

    void getDirectionIntersectionUnsafe(const glm::vec3 &origin, const glm::vec3 &dir_n,
        SimpleIntersection &out_intersection) override;

private:
    void update_bounds();

private:
    Mesh *mesh_{};
    Material *material_{};
};
