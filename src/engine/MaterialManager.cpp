#include "MaterialManager.h"

MaterialManager::MaterialManager()
    : AbstractManager<Material>("mat_")
{}

Material *MaterialManager::clone(Material *m, const char *name)
{
    std::string name_string = generate_or_check_name(name);
    return add(m->clone(), std::move(name_string));
}
