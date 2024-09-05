#include "ShaderManager.h"

ShaderManager::ShaderManager()
    : AbstractManager("shader_")
{}

void ShaderManager::recompileAll()
{
    for (auto &o : objects_)
    {
        o.obj->recompile();
    }
}