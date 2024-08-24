#include "Image.h"

// clang-format off
#define STB_IMAGE_IMPLEMENTATION
#include "EngineGlobals.h"
#include "stb_image.h"
#include "fs/FileSystem.h"

#include <iostream>
// clang-format on


Image::Image() = default;

Image::Image(const char *path, bool flip_y)
{
    load(path, flip_y);
}

Image::~Image()
{
    clear();
}

void Image::load(const char *path, bool flip_y)
{
    clear();

    stbi_set_flip_vertically_on_load(flip_y);
    std::string abs_path = engine_globals.fs->toAbsolutePath(path);

    data_ = stbi_load(abs_path.c_str(), &width_, &height_, &num_ch_, 0);
    if (!data_)
    {
        std::cout << "Failed to load image: " << abs_path << std::endl;
        clear();
    }

    switch (num_ch_)
    {
    case 1: format_ = Format::R; break;
    case 2: format_ = Format::RG; break;
    case 3: format_ = Format::RGB; break;
    case 4: format_ = Format::RGBA; break;
    default: format_ = Format::INVALID; break;
    }
}

void Image::clear()
{
    if (data_)
    {
        stbi_image_free(data_);
        data_ = nullptr;
        width_ = -1;
        height_ = -1;
        num_ch_ = -1;
        format_ = Format::INVALID;
    }
}