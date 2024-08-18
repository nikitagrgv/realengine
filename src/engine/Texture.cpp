#include "Texture.h"

#include "Image.h"
#include "glad/glad.h"

#include <iostream>

Texture::Texture(const Image &image, Format format)
{
    if (!image.isLoaded())
    {
        std::cout << "Failed to load texture" << std::endl;
        return;
    }

    int image_format = 0;
    if (image.getNumChannels() == 4)
    {
        image_format = GL_RGBA;
    }
    else if (image.getNumChannels() == 3)
    {
        image_format = GL_RGB;
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
        return;
    }

    int gl_format = 0;
    if (format == Format::RGBA)
    {
        gl_format = GL_RGBA;
    }
    else if (format == Format::RGB)
    {
        gl_format = GL_RGB;
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
        return;
    }

    glGenTextures(1, &id_);
    glBindTexture(GL_TEXTURE_2D, id_);
    glTexImage2D(GL_TEXTURE_2D, 0, gl_format, image.getWidth(), image.getHeight(), 0, image_format,
        GL_UNSIGNED_BYTE, image.getData());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture::~Texture()
{
    clear();
}

void Texture::bind() const
{
    glBindTexture(GL_TEXTURE_2D, id_);
}

void Texture::clear()
{
    if (id_ != 0)
    {
        glDeleteTextures(1, &id_);
        id_ = 0;
    }
}
