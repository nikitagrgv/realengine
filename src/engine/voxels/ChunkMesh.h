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
        float ao;
    };

    ChunkMesh();

    REALENGINE_INLINE void bind() const { vao.bind(); }
    REALENGINE_INLINE int getNumCpuVertices() const { return vbo.getNumCpuVertices(); }
    REALENGINE_INLINE int getNumGpuVertices() const { return vbo.getNumGpuVertices(); }
    REALENGINE_INLINE void clear() { vbo.clear(); }
    REALENGINE_INLINE void deallocate() { vbo.deallocate(); }
    REALENGINE_INLINE void flush() { vbo.flush(true); }
    REALENGINE_INLINE int addVertex(const Vertex &v) { return vbo.addVertex(v); }
    REALENGINE_INLINE void addRaw(const void *data, int size_bytes)
    {
        vbo.addRaw(data, size_bytes);
    }

private:
    VertexArrayObject vao;
    VertexBufferObject<Vertex> vbo;
};
