#pragma once

#include "Base.h"


class Image;
class Texture
{
public:
    enum class Format
    {
        RGBA,
        RGB
    };

    REMOVE_COPY_MOVE_CLASS(Texture);

    explicit Texture(const Image &image, Format format = Format::RGBA);
    ~Texture();

    void bind() const;

    void clear();

private:
    unsigned int id_{0};
};
