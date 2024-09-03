#include "ShaderManager.h"

#include "Shader.h"

#include <cassert>

Shader *ShaderManager::createShader(const char *name)
{
    Shader shader;
    return addShader(std::move(shader), name);
}

Shader *ShaderManager::addShader(Shader shader, const char *name)
{
    std::string name_string = name ? name : "";

    // TODO: shitty
    if (name_string.empty())
    {
        int i = 0;
        while (true)
        {
            name_string = "shader_";
            name_string += std::to_string(i);
            if (shaders_.find(name_string) == shaders_.end())
            {
                break;
            }
            i++;
        }
    }
    else
    {
        auto it = shaders_.find(name_string);
        assert(it == shaders_.end());
        if (it != shaders_.end())
        {
            return nullptr;
        }
    }

    auto unique_ptr = std::make_unique<Shader>(std::move(shader));
    auto ptr = unique_ptr.get();
    shaders_[name_string] = std::move(unique_ptr);
    shaders_names_[ptr] = std::move(name_string);
    return ptr;
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

void ShaderManager::removeShader(Shader *shader)
{
    auto name_it = shaders_names_.find(shader);
    if (name_it == shaders_names_.end())
    {
        return;
    }
    auto it = shaders_.find(name_it->second);
    assert(it != shaders_.end());
    shaders_names_.erase(name_it);
    shaders_.erase(it);
}