#pragma once

#include "Base.h"
#include "VertexArrayObject.h"
#include "VertexBufferObject.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

class ChunkMesh
{
public:
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 norm;
        glm::vec2 uv;
    };

    ChunkMesh();

    REALENGINE_INLINE void bind() const { vao.bind(); }
    REALENGINE_INLINE int getNumVertices() const { return vbo.getNumVertices(); }
    REALENGINE_INLINE void clear() { vbo.clear(); }
    REALENGINE_INLINE void flush() { vbo.flush(true); }
    REALENGINE_INLINE int addVertex(const Vertex &v) { return vbo.addVertex(v); }

private:
    VertexArrayObject vao;
    VertexBufferObject<Vertex> vbo;
};
