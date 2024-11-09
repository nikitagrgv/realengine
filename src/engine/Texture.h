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
        NearestMipmapNearest,
    };

    enum class FlipMode
    {
        DontFlip,
        FlipY
    };

    REMOVE_COPY_MOVE_CLASS(Texture);

    struct LoadParams
    {
        Format target_format = Format::RGBA;
        Wrap wrap = Wrap::Repeat;
        Filter min_filter = Filter::Linear;
        Filter mag_filter = Filter::Linear;
        int max_mipmap_level = 1000;
    };

    Texture();
    explicit Texture(const Image &image, const LoadParams &params = {});
    explicit Texture(const char *filename, FlipMode flip_mode = FlipMode::FlipY,
        const LoadParams &params = {});
    Texture(void *data, int width, int height, Format src_format, const LoadParams &params = {});
    ~Texture();

    void load(const char *filename, FlipMode flip_mode = FlipMode::FlipY,
        const LoadParams &params = {});
    void load(const Image &image, const LoadParams &params = {});
    void load(void *data, int width, int height, Format src_format, const LoadParams &params = {});

    void loadCubemap(const char **filenames, FlipMode flip_mode = FlipMode::FlipY,
        const LoadParams &params = {});
    void loadCubemap(const Image *images, const LoadParams &params = {});
    void loadCubemap(void **datas, const int *widths, const int *heights, const Format *src_formats,
        const LoadParams &params = {});

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
