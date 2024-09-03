#include "ShaderManager.h"

#include "Shader.h"

#include <cassert>

Shader *ShaderManager::createShader(const char *name)
{
    Shader shader;
    return addShader(std::move(shader), name);
}

Shader *ShaderManager::addShader(Shader material, const char *name)
{
    auto it = shaders_.find(name);
    assert(it == shaders_.end());
    if (it != shaders_.end())
    {
        return nullptr;
    }
    shaders_[name] = std::make_unique<Shader>(std::move(material));
    shaders_names_[shaders_[name].get()] = name;
    return shaders_[name].get();
}

Shader *ShaderManager::getShader(const char *name)
{
    auto it = shaders_.find(name);
    if (it == shaders_.end())
    {
        return nullptr;
    }
    return it->second.get();
}

void ShaderManager::removeShader(const char *name)
{
    auto it = shaders_.find(name);
    if (it == shaders_.end())
    {
        return;
    }
    auto name_it = shaders_names_.find(it->second.get());
    assert(name_it != shaders_names_.end());
    shaders_names_.erase(name_it);
    shaders_.erase(it);
}

void ShaderManager::removeShader(Shader *material)
{
    auto name_it = shaders_names_.find(material);
    if (name_it == shaders_names_.end())
    {
        return;
    }
    auto it = shaders_.find(name_it->second);
    assert(it != shaders_.end());
    shaders_names_.erase(name_it);
    shaders_.erase(it);
}