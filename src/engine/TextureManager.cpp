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
    auto it = textures_.find(name);
    assert(it == textures_.end());
    if (it != textures_.end())
    {
        return nullptr;
    }
    textures_[name] = std::make_unique<Texture>(std::move(texture));
    textures_names_[textures_[name].get()] = name;
    return textures_[name].get();
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