#pragma once
#include "Base.h"

#include <algorithm>
#include <iostream>
#include <vector>

class VertexBufferObjectHelper
{
protected:
    static unsigned int createBuffer();
    static void destroyBuffer(unsigned int vbo);
    static void bindBuffer(unsigned int vbo);
    static void setBufferData(size_t size, const void *data, bool dynamic);
    static void unbindBuffer();
};

template<typename V>
class VertexBufferObject : protected VertexBufferObjectHelper
{
public:
    REMOVE_COPY_CLASS(VertexBufferObject);

    static void unbind() { unbindBuffer(); }

    VertexBufferObject() { vbo_ = createBuffer(); }

    ~VertexBufferObject()
    {
        if (vbo_ != 0)
        {
            destroyBuffer(vbo_);
        }
    }

    VertexBufferObject(VertexBufferObject &&other) noexcept { *this = std::move(other); }

    VertexBufferObject &operator=(VertexBufferObject &&other) noexcept
    {
        if (this != &other)
        {
            if (vbo_ != 0)
            {
                destroyBuffer(vbo_);
            }
            vbo_ = other.vbo_;
            other.vbo_ = 0;
            vertices_ = std::move(other.vertices_);
        }
        return *this;
    }

    int addVertex(const V &v)
    {
        const int index = vertices_.size();
        vertices_.push_back(v);
        return index;
    }

    const V &getVertex(int index) const { return vertices_[index]; }
    V &getVertex(int index) { return vertices_[index]; }
    void setVertex(const V &v, int index) { vertices_[index] = v; }

    int getNumVertices() const { return vertices_.size(); }

    void clear() { vertices_.clear(); }

    void bind() const
    {
        if (vbo_ == 0)
        {
            std::cout << "VertexBufferObject::bind() - vbo_ == 0" << std::endl;
            return;
        }
        bindBuffer(vbo_);
    }

    void flush(bool dynamic = false)
    {
        if (vbo_ == 0)
        {
            std::cout << "VertexBufferObject::flush() - vbo_ == 0" << std::endl;
            return;
        }
        bindBuffer(vbo_);
        setBufferData(vertices_.size() * VERTEX_SIZE, vertices_.data(), dynamic);
    }

private:
    static constexpr int VERTEX_SIZE = sizeof(V);

    std::vector<V> vertices_;
    unsigned int vbo_{};
};
