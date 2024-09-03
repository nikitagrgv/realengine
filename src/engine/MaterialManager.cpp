#include "MaterialManager.h"

#include "Material.h"

#include <cassert>

Material *MaterialManager::createMaterial(const char *name)
{
    Material material;
    return addMaterial(std::move(material), name);
}

Material *MaterialManager::addMaterial(Material material, const char *name)
{
    std::string name_string = name ? name : "";

    // TODO: shitty
    if (name_string.empty())
    {
        int i = 0;
        while (true)
        {
            name_string = "mat_";
            name_string += std::to_string(i);
            if (materials_.find(name_string) == materials_.end())
            {
                break;
            }
            i++;
        }
    }
    else
    {
        auto it = materials_.find(name_string);
        assert(it == materials_.end());
        if (it != materials_.end())
        {
            return nullptr;
        }
    }

    auto unique_ptr = std::make_unique<Material>(std::move(material));
    auto ptr = unique_ptr.get();
    materials_[name_string] = std::move(unique_ptr);
    materials_names_[ptr] = std::move(name_string);
    return ptr;
}

Material *MaterialManager::getMaterial(const char *name)
{
    auto it = materials_.find(name);
    if (it == materials_.end())
    {
        return nullptr;
    }
    return it->second.get();
}

void MaterialManager::removeMaterial(const char *name)
{
    auto it = materials_.find(name);
    if (it == materials_.end())
    {
        return;
    }
    auto name_it = materials_names_.find(it->second.get());
    assert(name_it != materials_names_.end());
    materials_names_.erase(name_it);
    materials_.erase(it);
}

void MaterialManager::removeMaterial(Material *material)
{
    auto name_it = materials_names_.find(material);
    if (name_it == materials_names_.end())
    {
        return;
    }
    auto it = materials_.find(name_it->second);
    assert(it != materials_.end());
    materials_names_.erase(name_it);
    materials_.erase(it);
}