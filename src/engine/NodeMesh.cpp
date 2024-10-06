#include "NodeMesh.h"

#include "Mesh.h"

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
    update_global_bound_box();
}