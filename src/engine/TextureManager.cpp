#include "TextureManager.h"

#include "Texture.h"

#include <cassert>

Texture *TextureManager::createTexture(const char *name)
{
    Texture texture;
    return addTexture(std::move(texture), name);
}

Texture *TextureManager::addTexture(Texture texture, const char *name)
{
    std::string name_string = name ? name : "";

    // TODO: shitty
    if (name_string.empty())
    {
        int i = 0;
        while (true)
        {
            name_string = "texture_";
            name_string += std::to_string(i);
            if (textures_.find(name_string) == textures_.end())
            {
                break;
            }
            i++;
        }
    }
    else
    {
        auto it = textures_.find(name_string);
        assert(it == textures_.end());
        if (it != textures_.end())
        {
            return nullptr;
        }
    }

    auto unique_ptr = std::make_unique<Texture>(std::move(texture));
    auto ptr = unique_ptr.get();
    textures_[name_string] = std::move(unique_ptr);
    textures_names_[ptr] = std::move(name_string);
    return ptr;
}

Texture *TextureManager::getTexture(const char *name)
{
    auto it = textures_.find(name);
    if (it == textures_.end())
    {
        return nullptr;
    }
    return it->second.get();
}

void TextureManager::removeTexture(const char *name)
{
    auto it = textures_.find(name);
    if (it == textures_.end())
    {
        return;
    }
    auto name_it = textures_names_.find(it->second.get());
    assert(name_it != textures_names_.end());
    textures_names_.erase(name_it);
    textures_.erase(it);
}

void TextureManager::removeTexture(Texture *texture)
{
    auto name_it = textures_names_.find(texture);
    if (name_it == textures_names_.end())
    {
        return;
    }
    auto it = textures_.find(name_it->second);
    assert(it != textures_.end());
    textures_names_.erase(name_it);
    textures_.erase(it);
}