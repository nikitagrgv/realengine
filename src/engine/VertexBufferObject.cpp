#include "VertexBufferObject.h"

// clang-format off
#include <glad/glad.h>
// clang-format on

unsigned int VertexBufferObjectHelper::createBuffer()
{
    unsigned int vbo;
    GL_CHECKED(glGenBuffers(1, &vbo));
    return vbo;
}

void VertexBufferObjectHelper::destroyBuffer(unsigned int vbo)
{
    GL_CHECKED(glDeleteBuffers(1, &vbo));
}

void VertexBufferObjectHelper::bindBuffer(unsigned int vbo)
{
    GL_CHECKED(glBindBuffer(GL_ARRAY_BUFFER, vbo));
}

void VertexBufferObjectHelper::setBufferData(size_t size, const void *data, bool dynamic)
{
    const int load_flag = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    GL_CHECKED(glBufferData(GL_ARRAY_BUFFER, size, data, load_flag));
}

void VertexBufferObjectHelper::unbindBuffer()
{
    GL_CHECKED(glBindBuffer(GL_ARRAY_BUFFER, 0));
}