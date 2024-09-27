#pragma once

#include "Base.h"

#include "glm/vec4.hpp"

class Image
{
public:
    enum class Format
    {
        INVALID,
        RGBA,
        RGB,
        RG,
        R,
    };

    static int getNumChannels(Format format);

    REMOVE_COPY_CLASS(Image);

    Image();
    explicit Image(const char *path, bool flip_y = true);
    Image(int width, int height, Format format);

    ~Image();

    Image(Image &&other) noexcept;
    Image &operator=(Image &&other) noexcept;

    void load(const char *path, bool flip_y = true);
    void create(int width, int height, Format format);

    bool isLoaded() const { return data_ != nullptr; }

    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    int getNumChannels() const { return num_ch_; }
    Format getFormat() const { return format_; }

    unsigned char *getData() const { return data_; }

    void setPixelFloat(int x, int y, const glm::vec4 &pixel);
    glm::vec4 getPixelFloat(int x, int y) const;

    void setPixel(int x, int y, const glm::u8vec4 &pixel);
    glm::u8vec4 getPixel(int x, int y) const;

    void fill(const glm::vec4 &color);
    void fill(const glm::u8vec4 &color);

    void clear();

private:
    static unsigned char convert_to_u8_pixel(float float_pixel);
    static glm::u8vec4 convert_to_u8_pixel(glm::vec4 vec4_pixel);

    static float convert_to_float_pixel(unsigned char u8_pixel);
    static glm::vec4 convert_to_float_pixel(glm::u8vec4 u8_pixel);

private:
    unsigned char *data_{};
    int width_{-1};
    int height_{-1};
    int num_ch_{-1};
    Format format_{Format::INVALID};
};
