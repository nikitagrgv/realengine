#pragma once

#include "Base.h"

#include <vector>

class IndexBufferObject
{
public:
    REMOVE_COPY_CLASS(IndexBufferObject);

    static void unbind();

    IndexBufferObject();

    ~IndexBufferObject();

    IndexBufferObject(IndexBufferObject &&other) noexcept;

    IndexBufferObject &operator=(IndexBufferObject &&other) noexcept;

    int addIndex(unsigned int i);
    unsigned int getIndex(int i) const;

    int getNumIndices() const;

    void clear();

    void bind();

    void flush(bool dynamic = false);

private:
    static constexpr int INDEX_SIZE = sizeof(unsigned int);

    std::vector<unsigned int> indices_;
    unsigned int ebo_{0};
};