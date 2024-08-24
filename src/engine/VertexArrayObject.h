#pragma once

#include "Base.h"

#include <vector>

class VertexArrayObject
{
public:
    REMOVE_COPY_CLASS(VertexArrayObject);

    static void unbind();

    VertexArrayObject();
    ~VertexArrayObject();

    VertexArrayObject(VertexArrayObject &&other) noexcept;

    VertexArrayObject &operator=(VertexArrayObject &&other) noexcept;

    void addAttributeFloat(int count);

    void clear();

    void bind() const;

    void flush() const;

private:
    struct Attribute
    {
        int type{-1};
        int size_of_type{-1};
        int count{-1};
    };
    std::vector<Attribute> attributes_;
    unsigned int vao_{};
};