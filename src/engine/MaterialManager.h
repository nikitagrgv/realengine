#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class Material;

class MaterialManager
{
public:
    Material *createMaterial(const char *name);

    Material *addMaterial(Material material, const char *name);

    Material *getMaterial(const char *name);

    void removeMaterial(const char *name);
    void removeMaterial(Material *material);

private:
    std::unordered_map<std::string, std::unique_ptr<Material>> materials_;
    std::unordered_map<Material *, std::string> material_names_;
};