#include "Mesh.h"

Mesh::Mesh()
{
    vao_.bind();
    vao_.addAttributeFloat(3); // pos
    vao_.addAttributeFloat(3); // norm
    vao_.addAttributeFloat(2); // uv
    vbo_.bind();
    ebo_.bind();
    vao_.flush();
    VertexArrayObject::unbind();
}

int Mesh::addVertex()
{
    return vbo_.addVertex(Vertex{});
}

void Mesh::addVertices(int num_vertices)
{
    vbo_.addVertices(num_vertices);
}

int Mesh::getNumVertices() const
{
    return vbo_.getNumVertices();
}

int Mesh::addIndex(unsigned int v1)
{
    return ebo_.addIndex(v1);
}

void Mesh::addIndices(int num_indices)
{
    ebo_.addIndices(num_indices);
}

void Mesh::addIndices(unsigned int v1, unsigned int v2, unsigned int v3)
{
    ebo_.addIndex(v1);
    ebo_.addIndex(v2);
    ebo_.addIndex(v3);
}

int Mesh::getNumIndices() const
{
    return ebo_.getNumIndices();
}

void Mesh::clearIndices()
{
    ebo_.clear();
}

void Mesh::clear()
{
    vbo_.clear();
    ebo_.clear();
}

void Mesh::flush(bool dynamic)
{
    vbo_.flush(dynamic);
    ebo_.flush(dynamic);
}

void Mesh::bind()
{
    vao_.bind();
}