#include "Texture.h"

#include "Image.h"
#include "glad/glad.h"

#include <cassert>
#include <iostream>

namespace
{

bool format_to_gl_format(Texture::Format format, int &gl_format)
{
    switch (format)
    {
    case Texture::Format::RGB: gl_format = GL_RGB; break;
    case Texture::Format::RGBA: gl_format = GL_RGBA; break;
    default: return false;
    }
    return true;
}

bool wrap_to_gl_wrap(Texture::Wrap wrap, int &gl_wrap)
{
    switch (wrap)
    {
    case Texture::Wrap::Repeat: gl_wrap = GL_REPEAT; break;
    case Texture::Wrap::ClampToEdge: gl_wrap = GL_CLAMP_TO_EDGE; break;
    default: return false;
    }
    return true;
}

bool filter_to_gl_filter(Texture::Filter filter, int &gl_filter)
{
    switch (filter)
    {
    case Texture::Filter::Nearest: gl_filter = GL_NEAREST; break;
    case Texture::Filter::Linear: gl_filter = GL_LINEAR; break;
    case Texture::Filter::LinearMipmapNearest: gl_filter = GL_LINEAR_MIPMAP_NEAREST; break;
    case Texture::Filter::LinearMipmapLinear: gl_filter = GL_LINEAR_MIPMAP_LINEAR; break;
    default: return false;
    }
    return true;
}

Texture::Format image_format_to_texture_format(Image::Format format)
{
    switch (format)
    {
    case Image::Format::RGB: return Texture::Format::RGB;
    case Image::Format::RGBA: return Texture::Format::RGBA;
    default: return Texture::Format::Invalid;
    }
}

} // namespace

Texture::Texture() = default;

Texture::Texture(const Image &image, Format target_format, Wrap wrap, Filter min_filter,
    Filter mag_filter)
{
    load(image, target_format, wrap, min_filter, mag_filter);
}

Texture::Texture(const char *filename, Format target_format, Wrap wrap, Filter min_filter,
    Filter mag_filter)
{
    load(filename, target_format, wrap, min_filter, mag_filter);
}

Texture::Texture(void *data, int width, int height, Format src_format, Format target_format,
    Wrap wrap, Filter min_filter, Filter mag_filter)
{
    load(data, width, height, src_format, target_format, wrap, min_filter, mag_filter);
}

Texture::~Texture()
{
    clear();
}

void Texture::load(const char *filename, Format target_format, Wrap wrap, Filter min_filter,
    Filter mag_filter)
{
    clear();
    Image image(filename);
    if (!image.isLoaded())
    {
        std::cout << "Failed to load image" << std::endl;
        return;
    }
    load(image, target_format, wrap, min_filter, mag_filter);
}

void Texture::load(const Image &image, Format target_format, Wrap wrap, Filter min_filter,
    Filter mag_filter)
{
    clear();
    if (!image.isLoaded())
    {
        std::cout << "Failed to load texture" << std::endl;
        return;
    }

    const int width = image.getWidth();
    const int height = image.getHeight();
    void *data = image.getData();
    const Format src_format = image_format_to_texture_format(image.getFormat());
    load(data, width, height, src_format, target_format, wrap, min_filter, mag_filter);
}

void Texture::load(void *data, int width, int height, Format src_format, Format target_format,
    Wrap wrap, Filter min_filter, Filter mag_filter)
{
    clear();

    width_ = width;
    height_ = height;

    int gl_src_format = 0;
    if (!format_to_gl_format(src_format, gl_src_format))
    {
        std::cout << "Invalid format" << std::endl;
    }

    int gl_dst_format = 0;
    if (!format_to_gl_format(target_format, gl_dst_format))
    {
        std::cout << "Invalid format" << std::endl;
    }

    int gl_wrap = 0;
    if (!wrap_to_gl_wrap(wrap, gl_wrap))
    {
        std::cout << "Invalid wrap" << std::endl;
    }

    int gl_min_filter = 0;
    if (!filter_to_gl_filter(min_filter, gl_min_filter))
    {
        std::cout << "Invalid min filter" << std::endl;
    }

    int gl_mag_filter = 0;
    if (!filter_to_gl_filter(mag_filter, gl_mag_filter))
    {
        std::cout << "Invalid mag filter" << std::endl;
    }

    GL_CHECKED(glGenTextures(1, &id_));
    GL_CHECKED(glBindTexture(GL_TEXTURE_2D, id_));
    GL_CHECKED(glTexImage2D(GL_TEXTURE_2D, 0, gl_dst_format, width, height, 0, gl_src_format,
        GL_UNSIGNED_BYTE, data));
    if (gl_min_filter == GL_LINEAR_MIPMAP_LINEAR || gl_min_filter == GL_LINEAR_MIPMAP_NEAREST
        || gl_mag_filter == GL_LINEAR_MIPMAP_LINEAR || gl_mag_filter == GL_LINEAR_MIPMAP_NEAREST)
    {
        GL_CHECKED(glGenerateMipmap(GL_TEXTURE_2D));
    }
    GL_CHECKED(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gl_wrap));
    GL_CHECKED(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gl_wrap));
    GL_CHECKED(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_min_filter));
    GL_CHECKED(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_mag_filter));
    type_ = Type::Texture2D;
}

void Texture::loadCubemap(const char **filenames, Format target_format, Wrap wrap,
    Filter min_filter, Filter mag_filter)
{
    clear();
    Image images[6];
    for (int i = 0; i < 6; ++i)
    {
        Image &image = images[i];
        image.load(filenames[i]);
        if (!image.isLoaded())
        {
            std::cout << "Failed to load image" << std::endl;
            return;
        }
    }
    loadCubemap(images, target_format, wrap, min_filter, mag_filter);
}

void Texture::loadCubemap(const Image *images, Format target_format, Wrap wrap, Filter min_filter,
    Filter mag_filter)
{
    clear();
    for (int i = 0; i < 6; ++i)
    {
        if (!images[i].isLoaded())
        {
            std::cout << "Failed to load texture" << std::endl;
            return;
        }
    }

    int widths[6];
    int heights[6];
    void *datas[6];
    Format src_formats[6];
    for (int i = 0; i < 6; ++i)
    {
        const Image &image = images[i];
        widths[i] = image.getWidth();
        heights[i] = image.getHeight();
        datas[i] = image.getData();
        src_formats[i] = image_format_to_texture_format(image.getFormat());
    }
    loadCubemap(datas, widths, heights, src_formats, target_format, wrap, min_filter, mag_filter);
}

void Texture::loadCubemap(void **datas, const int *widths, const int *heights,
    const Format *src_formats, Format target_format, Wrap wrap, Filter min_filter,
    Filter mag_filter)
{
    clear();

    // TODO: fix?
    width_ = widths[0];
    height_ = heights[0];

    int gl_dst_format = 0;
    if (!format_to_gl_format(target_format, gl_dst_format))
    {
        std::cout << "Invalid format" << std::endl;
    }

    int gl_wrap = 0;
    if (!wrap_to_gl_wrap(wrap, gl_wrap))
    {
        std::cout << "Invalid wrap" << std::endl;
    }

    int gl_min_filter = 0;
    if (!filter_to_gl_filter(min_filter, gl_min_filter))
    {
        std::cout << "Invalid min filter" << std::endl;
    }

    int gl_mag_filter = 0;
    if (!filter_to_gl_filter(mag_filter, gl_mag_filter))
    {
        std::cout << "Invalid mag filter" << std::endl;
    }

    GL_CHECKED(glGenTextures(1, &id_));
    GL_CHECKED(glBindTexture(GL_TEXTURE_CUBE_MAP, id_));

    for (int i = 0; i < 6; ++i)
    {
        int gl_src_format = 0;
        if (!format_to_gl_format(src_formats[i], gl_src_format))
        {
            std::cout << "Invalid format" << std::endl;
        }

        const int width = widths[i];
        const int height = heights[i];
        const void *data = datas[i];

        GL_CHECKED(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, gl_dst_format, width, height,
            0, gl_src_format, GL_UNSIGNED_BYTE, data));
    }

    if (gl_min_filter == GL_LINEAR_MIPMAP_LINEAR || gl_min_filter == GL_LINEAR_MIPMAP_NEAREST
        || gl_mag_filter == GL_LINEAR_MIPMAP_LINEAR || gl_mag_filter == GL_LINEAR_MIPMAP_NEAREST)
    {
        GL_CHECKED(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));
    }
    GL_CHECKED(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, gl_wrap));
    GL_CHECKED(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, gl_wrap));
    GL_CHECKED(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, gl_min_filter));
    GL_CHECKED(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, gl_mag_filter));
    type_ = Type::Cubemap;
}

void Texture::bind(int slot) const
{
    if (id_ == 0)
    {
        std::cout << "Failed to bind texture" << std::endl;
        return;
    }
    assert(slot >= 0 && slot < 32);
    GL_CHECKED(glActiveTexture(GL_TEXTURE0 + slot));
    GL_CHECKED(glBindTexture(GL_TEXTURE_2D, id_));
}

void Texture::clear()
{
    type_ = Type::Invalid;
    width_ = -1;
    height_ = -1;
    if (id_ != 0)
    {
        GL_CHECKED(glDeleteTextures(1, &id_));
        id_ = 0;
    }
}
