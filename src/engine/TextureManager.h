#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class Texture;

class TextureManager
{
public:
    Texture *createTexture(const char *name = nullptr);

    Texture *addTexture(Texture texture, const char *name = nullptr);

    Texture *getTexture(const char *name);

    void removeTexture(const char *name);
    void removeTexture(Texture *texture);

private:
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures_;
    std::unordered_map<Texture *, std::string> textures_names_;
};