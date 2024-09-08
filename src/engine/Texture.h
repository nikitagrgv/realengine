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

    REMOVE_COPY_MOVE_CLASS(Texture);

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

    void load(const char *filename, Format target_format = Format::RGBA, Wrap wrap = Wrap::Repeat,
        Filter min_filter = Filter::Linear, Filter mag_filter = Filter::Linear);
    void load(const Image &image, Format target_format = Format::RGBA, Wrap wrap = Wrap::Repeat,
        Filter min_filter = Filter::Linear, Filter mag_filter = Filter::Linear);
    void load(void *data, int width, int height, Format src_format,
        Format target_format = Format::RGBA, Wrap wrap = Wrap::Repeat,
        Filter min_filter = Filter::Linear, Filter mag_filter = Filter::Linear);

    bool isLoaded() const { return id_ != 0; }

    void *getID() const { return (void *)(size_t)id_; }

    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    void bind(int slot = 0) const;

    void clear();

private:
    int width_{-1};
    int height_{-1};
    unsigned int id_{0};
};
