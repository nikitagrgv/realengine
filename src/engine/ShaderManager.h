#pragma once

#include "AbstractManager.h"
#include "Shader.h"

class ShaderManager : public AbstractManager<Shader>
{
public:
    ShaderManager();

    void recompileAll();

};