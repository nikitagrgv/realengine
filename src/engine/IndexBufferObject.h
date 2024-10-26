#pragma once

#include "Base.h"

#include <vector>

class IndexBufferObject
{
public:
    REMOVE_COPY_MOVE_CLASS(IndexBufferObject);

    static void unbind();

    IndexBufferObject();

    ~IndexBufferObject();

    void addIndices(int num_indices);

    int addIndex(unsigned int i);
    unsigned int getIndex(int i) const;
    void setIndex(int i, unsigned int index);

    int getNumIndices() const;

    void clear();

    void bind();

    void flush(bool dynamic = false);

private:
    static constexpr int INDEX_SIZE = sizeof(unsigned int);

    std::vector<unsigned int> indices_;
    unsigned int ebo_{0};
};