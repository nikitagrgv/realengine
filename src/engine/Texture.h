#pragma once

#include "Base.h"

#include "glm/vec2.hpp"


class Image;
class Texture
{
public:
    enum class Type
    {
        Invalid,
        Texture2D,
        Cubemap,
    };

    enum class Format
    {
        Invalid,
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

    enum class FlipMode
    {
        DontFlip,
        FlipY
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
        Filter min_filter = Filter::Linear, Filter mag_filter = Filter::Linear,
        FlipMode flip_mode = FlipMode::FlipY);
    void load(const Image &image, Format target_format = Format::RGBA, Wrap wrap = Wrap::Repeat,
        Filter min_filter = Filter::Linear, Filter mag_filter = Filter::Linear);
    void load(void *data, int width, int height, Format src_format,
        Format target_format = Format::RGBA, Wrap wrap = Wrap::Repeat,
        Filter min_filter = Filter::Linear, Filter mag_filter = Filter::Linear);

    void loadCubemap(const char **filenames, Format target_format = Format::RGBA,
        Wrap wrap = Wrap::Repeat, Filter min_filter = Filter::Linear,
        Filter mag_filter = Filter::Linear, FlipMode flip_mode = FlipMode::FlipY);
    void loadCubemap(const Image *images, Format target_format = Format::RGBA,
        Wrap wrap = Wrap::Repeat, Filter min_filter = Filter::Linear,
        Filter mag_filter = Filter::Linear);
    void loadCubemap(void **datas, const int *widths, const int *heights, const Format *src_formats,
        Format target_format = Format::RGBA, Wrap wrap = Wrap::Repeat,
        Filter min_filter = Filter::Linear, Filter mag_filter = Filter::Linear);

    bool isLoaded() const { return id_ != 0; }

    void *getID() const { return (void *)(size_t)id_; }

    glm::ivec2 getSize() const { return glm::ivec2(width_, height_); }

    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    Type getType() const { return type_; }

    void bind(int slot = 0) const;

    void clear();

private:
    int width_{-1};
    int height_{-1};
    Type type_{Type::Invalid};
    unsigned int id_{0};
};
