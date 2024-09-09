#include "MaterialManager.h"

MaterialManager::MaterialManager()
    : AbstractManager<Material>("mat_")
{}

Material *MaterialManager::clone(Material *m, const char *name)
{
    std::string name_string = generate_or_check_name(name);
    UPtr<Material> cloned = m->clone();
    return add(std::move(cloned), std::move(name_string));
}

Material *MaterialManager::inherit(Material *m, const char *name)
{
    std::string name_string = generate_or_check_name(name);
    UPtr<Material> inherited = m->inherit();
    return add(std::move(inherited), std::move(name_string));
}
