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
    auto it = materials_.find(name);
    assert(it == materials_.end());
    if (it != materials_.end())
    {
        return nullptr;
    }
    materials_[name] = std::make_unique<Material>(std::move(material));
    material_names_[materials_[name].get()] = name;
    return materials_[name].get();
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
    auto name_it = material_names_.find(it->second.get());
    assert(name_it != material_names_.end());
    material_names_.erase(name_it);
    materials_.erase(it);
}

void MaterialManager::removeMaterial(Material *material)
{
    auto name_it = material_names_.find(material);
    if (name_it == material_names_.end())
    {
        return;
    }
    auto it = materials_.find(name_it->second);
    assert(it != materials_.end());
    material_names_.erase(name_it);
    materials_.erase(it);
}