#include "NodeMesh.h"

#include "Mesh.h"

NodeMesh::NodeMesh(int id)
    : Node(id, getTypeStatic())
{}

void NodeMesh::setMesh(Mesh *mesh)
{
    mesh_ = mesh;
    update_bounds();
}

void NodeMesh::update_bounds()
{
    bound_box_.min = glm::vec3{0.0f};
    bound_box_.max = glm::vec3{0.0f};

    if (!mesh_)
    {
        update_global_bound_box();
        return;
    }

    for (int i = 0, count = mesh_->getNumIndices(); i < count; ++i)
    {
        const int vertex = mesh_->getIndex(i);
        bound_box_.expand(mesh_->getVertexPos(vertex));
    }

    update_global_bound_box();
}