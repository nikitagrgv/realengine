#include "NodeMesh.h"

#include "Intersection.h"
#include "Mesh.h"
#include "math/Math.h"

NodeMesh::NodeMesh(int id)
    : Node(id, getTypeStatic())
{}

Mesh *NodeMesh::takeMesh()
{
    auto mesh = mesh_;
    setMesh(nullptr);
    return mesh;
}

void NodeMesh::setMesh(Mesh *mesh)
{
    if (mesh == mesh_)
    {
        return;
    }
    mesh_ = mesh;
    update_bounds();
}

void NodeMesh::getDirectionIntersection(const glm::vec3 &origin, const glm::vec3 &direction,
    SimpleIntersection &out_intersection)
{
    // Direction can be non normalized
    getDirectionIntersectionUnsafe(origin, direction, out_intersection);
}

void NodeMesh::getDirectionIntersectionUnsafe(const glm::vec3 &origin, const glm::vec3 &dir_n,
    SimpleIntersection &out_intersection)
{
    if (!mesh_)
    {
        out_intersection.clear();
        return;
    }
    const glm::vec3 loc_origin = getITransform() * glm::vec4(origin, 1.0f);
    const glm::vec3 loc_direction = getITransform() * glm::vec4(dir_n, 0.0f);
    const glm::vec3 loc_dir_n = glm::normalize(loc_direction);
    mesh_->getDirectionIntersectionUnsafe(loc_origin, loc_dir_n, out_intersection);
}

void NodeMesh::update_bounds()
{
    if (mesh_)
    {
        bound_box_ = mesh_->getBoundBox();
    }
    else
    {
        bound_box_.clear();
    }
    update_caches();
}