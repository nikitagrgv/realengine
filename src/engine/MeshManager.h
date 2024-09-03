#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class Mesh;

class MeshManager
{
public:
    Mesh *createMesh(const char *name = nullptr);

    Mesh *addMesh(Mesh mesh, const char *name = nullptr);

    Mesh *getMesh(const char *name);

    void removeMesh(const char *name);
    void removeMesh(Mesh *mesh);

private:
    std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes_;
    std::unordered_map<Mesh *, std::string> meshes_names_;
};