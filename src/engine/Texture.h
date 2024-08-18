#pragma once

#include "Base.h"


class Image;
class Texture
{
public:
    REMOVE_COPY_MOVE_CLASS(Texture);

    explicit Texture(const Image& image);
    ~Texture();

    void bind() const;

    void clear();

private:
    unsigned int id_{0};
};
