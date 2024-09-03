#include "MeshManager.h"

#include "Mesh.h"

#include <cassert>

Mesh *MeshManager::createMesh(const char *name)
{
    Mesh mesh;
    return addMesh(std::move(mesh), name);
}

Mesh *MeshManager::addMesh(Mesh mesh, const char *name)
{
    auto it = meshes_.find(name);
    assert(it == meshes_.end());
    if (it != meshes_.end())
    {
        return nullptr;
    }
    meshes_[name] = std::make_unique<Mesh>(std::move(mesh));
    meshes_names_[meshes_[name].get()] = name;
    return meshes_[name].get();
}

Mesh *MeshManager::getMesh(const char *name)
{
    auto it = meshes_.find(name);
    if (it == meshes_.end())
    {
        return nullptr;
    }
    return it->second.get();
}

void MeshManager::removeMesh(const char *name)
{
    auto it = meshes_.find(name);
    if (it == meshes_.end())
    {
        return;
    }
    auto name_it = meshes_names_.find(it->second.get());
    assert(name_it != meshes_names_.end());
    meshes_names_.erase(name_it);
    meshes_.erase(it);
}

void MeshManager::removeMesh(Mesh *mesh)
{
    auto name_it = meshes_names_.find(mesh);
    if (name_it == meshes_names_.end())
    {
        return;
    }
    auto it = meshes_.find(name_it->second);
    assert(it != meshes_.end());
    meshes_names_.erase(name_it);
    meshes_.erase(it);
}