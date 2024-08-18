#include "Image.h"

// clang-format off
#define STB_IMAGE_IMPLEMENTATION
#include "EngineGlobals.h"
#include "stb_image.h"
#include "fs/FileSystem.h"

#include <iostream>
// clang-format on


Image::Image(const char *path, bool flip_y)
{
    stbi_set_flip_vertically_on_load(flip_y);
    std::string abs_path = engine_globals.fs->toAbsolutePath(path);

    data_ = stbi_load(abs_path.c_str(), &width_, &height_, &num_ch_, 0);
    if (!data_)
    {
        std::cout << "Failed to load image: " << abs_path << std::endl;
        clear();
    }
}

Image::~Image()
{
    clear();
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
    }
}