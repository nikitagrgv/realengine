#include "NodeMesh.h"

NodeMesh::NodeMesh(int id)
    : Node(id, getTypeStatic())
{}

void NodeMesh::setMesh(Mesh *mesh)
{
    mesh_ = mesh;
}