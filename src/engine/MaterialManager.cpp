#include "MaterialManager.h"

#include "utils/Algos.h"

MaterialManager::MaterialManager()
    : AbstractManager<Material>("mat_")
{}

MaterialManager::~MaterialManager()
{
    // TODO# SHIT
    while (!objects_.empty())
    {
        Alg::removeIf(objects_, [](const Object &mat) { return mat.obj->getNumChildren() == 0; });
    }
}

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
