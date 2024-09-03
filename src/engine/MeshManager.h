#pragma once

#include "AbstractManager.h"
#include "Mesh.h"

class MeshManager : public AbstractManager<Mesh>
{
public:
    MeshManager();
};