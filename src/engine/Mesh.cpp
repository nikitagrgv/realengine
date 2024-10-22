#include "Mesh.h"

#include "Intersection.h"
#include "math/IntersectionMath.h"
#include "math/Math.h"

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

void Mesh::getDirectionIntersection(const glm::vec3 &origin, const glm::vec3 &direction,
    SimpleIntersection &out_intersection) const
{
    const glm::vec3 dir_n = glm::normalize(direction);
    getDirectionIntersectionUnsafe(origin, dir_n, out_intersection);
}

void Mesh::getDirectionIntersectionUnsafe(const glm::vec3 &origin, const glm::vec3 &dir_n,
    SimpleIntersection &out_intersection) const
{
    assert(math::isNormalized(dir_n));

    SimpleIntersection intersection;

    const int num_indices = ebo_.getNumIndices();

    if (num_indices == 0)
    {
        out_intersection = intersection;
        return;
    }

    assert(num_indices % 3 == 0);
    for (int i = 0; i < num_indices; i += 3)
    {
        const int i0 = ebo_.getIndex(i);
        const int i1 = ebo_.getIndex(i + 1);
        const int i2 = ebo_.getIndex(i + 2);
        const glm::vec3 p0 = vbo_.getVertex(i0).pos;
        const glm::vec3 p1 = vbo_.getVertex(i1).pos;
        const glm::vec3 p2 = vbo_.getVertex(i2).pos;

        SimpleIntersection si;
        math::getDirectionTriangleIntersectionUnsafe(origin, dir_n, p0, p1, p2, si);
        intersection.takeCloser(si);
    }

    out_intersection = intersection;
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
    update_bounds();
}

void Mesh::bind() const
{
    vao_.bind();
}

void Mesh::update_bounds()
{
    bound_box_.min = glm::vec3{0.0f};
    bound_box_.max = glm::vec3{0.0f};
    for (int i = 0, count = getNumIndices(); i < count; ++i)
    {
        const int vertex = getIndex(i);
        bound_box_.expand(getVertexPos(vertex));
    }
}