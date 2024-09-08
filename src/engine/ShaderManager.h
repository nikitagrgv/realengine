#pragma once

#include "AbstractManager.h"
#include "ShaderSource.h"

class ShaderManager : public AbstractManager<ShaderSource>
{
public:
    ShaderManager();

    void refreshAll();
};