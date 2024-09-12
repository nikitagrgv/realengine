#include "VertexArrayObject.h"

// clang-format off
#include <iostream>
#include <glad/glad.h>
// clang-format on

void VertexArrayObject::unbind()
{
    GL_CHECKED(glBindVertexArray(0));
}

VertexArrayObject::VertexArrayObject()
{
    GL_CHECKED(glGenVertexArrays(1, &vao_));
}

VertexArrayObject::~VertexArrayObject()
{
    if (vao_ != 0)
    {
        GL_CHECKED(glDeleteVertexArrays(1, &vao_));
    }
}

VertexArrayObject::VertexArrayObject(VertexArrayObject &&other) noexcept
{
    *this = std::move(other);
}

VertexArrayObject &VertexArrayObject::operator=(VertexArrayObject &&other) noexcept
{
    if (this != &other)
    {
        if (vao_ != 0)
        {
            GL_CHECKED(glDeleteVertexArrays(1, &vao_));
        }
        vao_ = other.vao_;
        other.vao_ = 0;
        attributes_ = std::move(other.attributes_);
    }
    return *this;
}

void VertexArrayObject::addAttributeFloat(int count)
{
    Attribute attr;
    attr.count = count;
    attr.size_of_type = sizeof(float);
    attr.type = GL_FLOAT;
    attributes_.push_back(attr);
}

void VertexArrayObject::clear()
{
    attributes_.clear();
}

void VertexArrayObject::bind() const
{
    if (vao_ == 0)
    {
        std::cout << "VertexArrayObject::bind() - vao_ == 0" << std::endl;
        return;
    }
    GL_CHECKED(glBindVertexArray(vao_));
}

void VertexArrayObject::flush() const
{
    if (vao_ == 0)
    {
        std::cout << "VertexArrayObject::flush() - vao_ == 0" << std::endl;
        return;
    }

    int stride = 0;
    for (const Attribute &attribute : attributes_)
    {
        stride += attribute.count * attribute.size_of_type;
    }

    GL_CHECKED(glBindVertexArray(vao_));
    size_t offset = 0;
    for (int i = 0; i < attributes_.size(); ++i)
    {
        const Attribute &attribute = attributes_[i];
        GL_CHECKED(glVertexAttribPointer(i, attribute.count, attribute.type, GL_FALSE, stride,
            reinterpret_cast<void *>(offset)));
        GL_CHECKED(glEnableVertexAttribArray(i));
        offset += attribute.count * attribute.size_of_type;
    }
    unbind();
}
