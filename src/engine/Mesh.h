#pragma once

#include "IndexBufferObject.h"
#include "VertexArrayObject.h"
#include "VertexBufferObject.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

class Mesh
{
public:
    Mesh();

    // Vertices
    int addVertex();

    int addVertex(const glm::vec3 &pos, const glm::vec3 &norm, const glm::vec2 &uv)
    {
        return vbo_.addVertex(Vertex{pos, norm, uv});
    }

    int addVertex(const glm::vec3 &pos, const glm::vec3 &norm)
    {
        return vbo_.addVertex(Vertex{pos, norm, glm::vec2{}});
    }

    int addVertex(const glm::vec3 &pos)
    {
        return vbo_.addVertex(Vertex{pos, glm::vec3{}, glm::vec2{}});
    }

    void addVertices(int num_vertices);

    glm::vec3 getVertexPos(int index) const { return vbo_.getVertex(index).pos; }
    void setVertexPos(int index, const glm::vec3 &pos) { vbo_.getVertex(index).pos = pos; }

    glm::vec3 getVertexNormal(int index) const { return vbo_.getVertex(index).norm; }
    void setVertexNormal(int index, const glm::vec3 &norm) { vbo_.getVertex(index).norm = norm; }

    glm::vec2 getVertexUV(int index) const { return vbo_.getVertex(index).uv; }
    void setVertexUV(int index, const glm::vec2 &uv) { vbo_.getVertex(index).uv = uv; }

    int getNumVertices() const;

    // Indices
    int addIndex(unsigned int v1);
    void addIndices(int num_indices);
    void addIndices(unsigned int v1, unsigned int v2, unsigned int v3);

    void setIndex(int i, unsigned int index) { ebo_.setIndex(i, index); }

    int getNumIndices() const;

    void clearIndices();

    // Mesh
    void clear();

    void flush(bool dynamic = false);

    void bind();

protected:
    struct Vertex
    {
        glm::vec3 pos{0.0f};
        glm::vec3 norm{0.0f};
        glm::vec2 uv{0.0f};
    };
    VertexBufferObject<Vertex> vbo_;
    VertexArrayObject vao_;
    IndexBufferObject ebo_;
};