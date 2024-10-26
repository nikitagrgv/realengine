#pragma once

#include "Base.h"
#include "VertexArrayObject.h"
#include "VertexBufferObject.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct ChunkMesh
{
public:
    ChunkMesh();

    void bind() const { vao.bind(); }
    int getNumVertices() const { return vbo.getNumVertices(); }
    void clear() { vbo.clear(); }

public:
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 norm;
        glm::vec2 uv;
    };
    VertexArrayObject vao;
    VertexBufferObject<Vertex> vbo;
};
