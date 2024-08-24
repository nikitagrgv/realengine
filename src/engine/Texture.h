#pragma once

#include "Base.h"


class Image;
class Texture
{
public:
    enum class Format
    {
        INVALID,
        RGBA,
        RGB
    };

    enum class Wrap
    {
        Repeat,
        ClampToEdge
    };

    enum class Filter
    {
        Nearest,
        Linear,
        LinearMipmapNearest,
        LinearMipmapLinear,
    };

    REMOVE_COPY_CLASS(Texture);

    Texture();
    explicit Texture(const Image &image, Format target_format = Format::RGBA,
        Wrap wrap = Wrap::Repeat, Filter min_filter = Filter::Linear,
        Filter mag_filter = Filter::Linear);
    explicit Texture(const char *filename, Format target_format = Format::RGBA,
        Wrap wrap = Wrap::Repeat, Filter min_filter = Filter::Linear,
        Filter mag_filter = Filter::Linear);
    Texture(void *data, int width, int height, Format src_format,
        Format target_format = Format::RGBA, Wrap wrap = Wrap::Repeat,
        Filter min_filter = Filter::Linear, Filter mag_filter = Filter::Linear);
    ~Texture();

    Texture(Texture &&other) noexcept;
    Texture &operator=(Texture &&other) noexcept;

    void load(const char *filename, Format target_format = Format::RGBA, Wrap wrap = Wrap::Repeat,
        Filter min_filter = Filter::Linear, Filter mag_filter = Filter::Linear);
    void load(const Image &image, Format target_format = Format::RGBA, Wrap wrap = Wrap::Repeat,
        Filter min_filter = Filter::Linear, Filter mag_filter = Filter::Linear);
    void load(void *data, int width, int height, Format src_format,
        Format target_format = Format::RGBA, Wrap wrap = Wrap::Repeat,
        Filter min_filter = Filter::Linear, Filter mag_filter = Filter::Linear);

    void bind() const;

    void clear();

private:
    unsigned int id_{0};
};
