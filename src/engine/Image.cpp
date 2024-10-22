#include "Image.h"

// clang-format off
#define STB_IMAGE_IMPLEMENTATION
#include "EngineGlobals.h"
#include "stb_image.h"
#include "fs/FileSystem.h"

#include <iostream>
// clang-format on


int Image::getNumChannels(Format format)
{
    switch (format)
    {
    case Format::R: return 1;
    case Format::RG: return 2;
    case Format::RGB: return 3;
    case Format::RGBA: return 4;
    default: assert(0); return 0;
    }
}

Image::Image() = default;

Image::Image(const char *path, FlipMode flip_mode)
{
    load(path, flip_mode);
}

Image::Image(int width, int height, Format format)
{
    create(width, height, format);
}

Image::~Image()
{
    clear();
}


Image::Image(Image &&other) noexcept
{
    *this = std::move(other);
}

Image &Image::operator=(Image &&other) noexcept
{
    if (this != &other)
    {
        clear();
        data_ = other.data_;
        width_ = other.width_;
        height_ = other.height_;
        num_ch_ = other.num_ch_;
        format_ = other.format_;
        other.data_ = nullptr;
        other.width_ = -1;
        other.height_ = -1;
        other.num_ch_ = -1;
        other.format_ = Format::INVALID;
    }
    return *this;
}

void Image::load(const char *path, FlipMode flip_mode)
{
    clear();

    stbi_set_flip_vertically_on_load((int)flip_mode & (int)FlipMode::FlipY);
    std::string abs_path = eng.fs->toAbsolutePath(path);

    unsigned char *stbi_data = stbi_load(abs_path.c_str(), &width_, &height_, &num_ch_, 0);
    if (!stbi_data)
    {
        std::cout << "Failed to load image: " << abs_path << std::endl;
        clear();
        return;
    }

    assert(!data_);

    const int data_size = num_ch_ * width_ * height_;
    data_ = new unsigned char[data_size];
    memcpy(data_, stbi_data, data_size);

    stbi_image_free(stbi_data);

    switch (num_ch_)
    {
    case 1: format_ = Format::R; break;
    case 2: format_ = Format::RG; break;
    case 3: format_ = Format::RGB; break;
    case 4: format_ = Format::RGBA; break;
    default: format_ = Format::INVALID; break;
    }
}

void Image::create(int width, int height, Format format)
{
    assert(width > 0 && height > 0 && getNumChannels(format) != 0);

    clear();

    width_ = width;
    height_ = height;
    format_ = format;
    num_ch_ = getNumChannels(format_);

    const int data_size = num_ch_ * width_ * height_;

    data_ = new unsigned char[data_size];

    switch (format_)
    {
    case Format::R:
    case Format::RG:
    case Format::RGB:
    {
        memset(data_, 0, data_size);
        break;
    }
    case Format::RGBA:
    {
        const int num_pixels = width_ * height_;
        for (int i = 0; i < num_pixels; ++i)
        {
            data_[i * 4 + 0] = 0;
            data_[i * 4 + 1] = 0;
            data_[i * 4 + 2] = 0;
            data_[i * 4 + 3] = 0xFF;
        }
        break;
    }
    default: assert(0); break;
    }
}

void Image::setPixelFloat(int x, int y, const glm::vec4 &pixel)
{
    assert(isLoaded());
    const glm::u8vec4 u8_pixel = convert_to_u8_pixel(pixel);
    setPixel(x, y, u8_pixel);
}

glm::vec4 Image::getPixelFloat(int x, int y) const
{
    assert(isLoaded());
    const glm::u8vec4 u8_pixel = getPixel(x, y);
    return convert_to_float_pixel(u8_pixel);
}

void Image::setPixel(int x, int y, const glm::u8vec4 &pixel)
{
    const int pixel_size = num_ch_;
    const int row_size = width_ * pixel_size;
    const int pos = row_size * y + pixel_size * x;

    unsigned char *p = &data_[pos];

    memcpy(p, &pixel, num_ch_);
}

glm::u8vec4 Image::getPixel(int x, int y) const
{
    const int pixel_size = num_ch_;
    const int row_size = width_ * pixel_size;
    const int pos = row_size * y + pixel_size * x;

    const unsigned char *p = &data_[pos];

    glm::u8vec4 u8_pixel{0, 0, 0, 1};
    memcpy(&u8_pixel, p, num_ch_);
    return u8_pixel;
}

void Image::fill(const glm::vec4 &color)
{
    const glm::u8vec4 u8_color = convert_to_u8_pixel(color);
    fill(u8_color);
}

void Image::fill(const glm::u8vec4 &color)
{
    for (int y = 0; y < height_; ++y)
    {
        for (int x = 0; x < width_; ++x)
        {
            setPixel(x, y, color);
        }
    }
}

void Image::clear()
{
    if (data_)
    {
        delete[] data_;
        data_ = nullptr;
        width_ = -1;
        height_ = -1;
        num_ch_ = -1;
        format_ = Format::INVALID;
    }
}

unsigned char Image::convert_to_u8_pixel(float float_pixel)
{
    assert(float_pixel >= 0 && float_pixel <= 1);
    return static_cast<unsigned char>(float_pixel * 255.0f);
}

glm::u8vec4 Image::convert_to_u8_pixel(glm::vec4 vec4_pixel)
{
    // TODO: optimize?
    glm::u8vec4 ret;
    ret.r = convert_to_u8_pixel(vec4_pixel.r);
    ret.g = convert_to_u8_pixel(vec4_pixel.g);
    ret.b = convert_to_u8_pixel(vec4_pixel.b);
    ret.a = convert_to_u8_pixel(vec4_pixel.a);
    return ret;
}

float Image::convert_to_float_pixel(unsigned char u8_pixel)
{
    return static_cast<float>(u8_pixel) / 255.0f;
}

glm::vec4 Image::convert_to_float_pixel(glm::u8vec4 u8_pixel)
{
    glm::vec4 ret;
    ret.r = convert_to_float_pixel(u8_pixel.r);
    ret.g = convert_to_float_pixel(u8_pixel.g);
    ret.b = convert_to_float_pixel(u8_pixel.b);
    ret.a = convert_to_float_pixel(u8_pixel.a);
    return ret;
}