#include "IndexBufferObject.h"

// clang-format off
#include "glad/glad.h"
// clang-format on

#include <algorithm>
#include <iostream>

void IndexBufferObject::unbind()
{
    GL_CHECKED(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

IndexBufferObject::IndexBufferObject()
{
    GL_CHECKED(glGenBuffers(1, &ebo_));
}

IndexBufferObject::~IndexBufferObject()
{
    GL_CHECKED(glDeleteBuffers(1, &ebo_));
}

void IndexBufferObject::addIndices(int num_indices)
{
    indices_.resize(indices_.size() + num_indices, 0);
}

int IndexBufferObject::addIndex(unsigned int i)
{
    const int index = indices_.size();
    indices_.push_back(i);
    return index;
}

unsigned int IndexBufferObject::getIndex(int i) const
{
    return indices_[i];
}

void IndexBufferObject::setIndex(int i, unsigned int index)
{
    indices_[i] = index;
}

int IndexBufferObject::getNumIndices() const
{
    return indices_.size();
}

void IndexBufferObject::clear()
{
    indices_.clear();
}

void IndexBufferObject::bind()
{
    if (ebo_ == 0)
    {
        std::cout << "IndexBufferObject::bind() - ebo_ == 0" << std::endl;
        return;
    }
    GL_CHECKED(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_));
}

void IndexBufferObject::flush(bool dynamic)
{
    if (ebo_ == 0)
    {
        std::cout << "IndexBufferObject::flush() - ebo_ == 0" << std::endl;
        return;
    }
    const int load_flag = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
    GL_CHECKED(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_));
    GL_CHECKED(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * INDEX_SIZE, indices_.data(), load_flag));
}
