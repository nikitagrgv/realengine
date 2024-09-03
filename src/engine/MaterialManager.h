#pragma once

#include "AbstractManager.h"
#include "Material.h"

class MaterialManager : public AbstractManager<Material>
{
public:
    MaterialManager();
};