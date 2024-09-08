#include "ShaderManager.h"

ShaderManager::ShaderManager()
    : AbstractManager("shader_")
{}

void ShaderManager::refreshAll()
{
    for (auto &o : objects_)
    {
        o.obj->refresh();
    }
}