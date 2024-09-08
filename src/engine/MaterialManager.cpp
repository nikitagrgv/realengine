#include "MaterialManager.h"

MaterialManager::MaterialManager()
    : AbstractManager<Material>("mat_")
{}

Material *MaterialManager::clone(Material *m, const char *name)
{
    std::string name_string = generate_or_check_name(name);
    auto cloned = makeU<Material>();
    m->cloneTo(*cloned);
    return add(std::move(cloned), std::move(name_string));
}
