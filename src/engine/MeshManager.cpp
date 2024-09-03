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
    std::string name_string = name ? name : "";

    // TODO: shitty
    if (name_string.empty())
    {
        int i = 0;
        while (true)
        {
            name_string = "mesh_";
            name_string += std::to_string(i);
            if (meshes_.find(name_string) == meshes_.end())
            {
                break;
            }
            i++;
        }
    }
    else
    {
        auto it = meshes_.find(name_string);
        assert(it == meshes_.end());
        if (it != meshes_.end())
        {
            return nullptr;
        }
    }

    auto unique_ptr = std::make_unique<Mesh>(std::move(mesh));
    auto ptr = unique_ptr.get();
    meshes_[name_string] = std::move(unique_ptr);
    meshes_names_[ptr] = std::move(name_string);
    return ptr;
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