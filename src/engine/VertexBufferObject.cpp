#include "VertexBufferObject.h"

// clang-format off
#include <glad/glad.h>
// clang-format on

unsigned int VertexBufferObjectHelper::createBuffer()
{
    unsigned int vbo;
    glGenBuffers(1, &vbo);
    return vbo;
}

void VertexBufferObjectHelper::destroyBuffer(unsigned int vbo)
{
    glDeleteBuffers(1, &vbo);
}

void VertexBufferObjectHelper::bindBuffer(unsigned int vbo)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

void VertexBufferObjectHelper::setBufferData(size_t size, const void *data, bool dynamic)
{
    const int load_flag = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    glBufferData(GL_ARRAY_BUFFER, size, data, load_flag);
}

void VertexBufferObjectHelper::unbindBuffer()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}