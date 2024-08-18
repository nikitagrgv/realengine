#include "MeshLoader.h"

#include "EngineGlobals.h"
#include "OBJ_Loader.h"
#include "fs/FileSystem.h"

MeshLoader::MeshLoader(const char *path)
{
    objl::Loader loader;
    if (!loader.LoadFile(engine_globals.fs->toAbsolutePath(path)))
    {
        std::cout << "Failed to load mesh: " << path << std::endl;
        return;
    }

    if (loader.LoadedMeshes.empty())
    {
        std::cout << "No meshes found: " << path << std::endl;
        return;
    }

    if (loader.LoadedMeshes.size() > 1)
    {
        std::cout << "Warning: multiple meshes in: " << path << std::endl;
    }

    const auto add_mesh = [this](const objl::Mesh &mesh) {
        for (const objl::Vertex &vertex : mesh.Vertices)
        {
            Vertex v;
            v.position = glm::vec3(vertex.Position.X, vertex.Position.Y, vertex.Position.Z);
            v.texture_coords = glm::vec2(vertex.TextureCoordinate.X, vertex.TextureCoordinate.Y);
            v.normal = glm::vec3(vertex.Normal.X, vertex.Normal.Y, vertex.Normal.Z);
            vertices_.push_back(v);
        }
        for (const unsigned int &index : mesh.Indices)
        {
            indices_.push_back(index);
        }
    };

    // TODO: multiple meshes
    add_mesh(loader.LoadedMeshes[0]);

    is_loaded_ = true;
}

int MeshLoader::getNumVertices() const
{
    return vertices_.size();
}

glm::vec3 MeshLoader::getVertexPosition(int index) const
{
    return vertices_[index].position;
}

glm::vec2 MeshLoader::getVertexTextureCoords(int index) const
{
    return vertices_[index].texture_coords;
}

glm::vec3 MeshLoader::getVertexNormal(int index) const
{
    return vertices_[index].normal;
}

int MeshLoader::getNumIndices() const
{
    return indices_.size();
}

unsigned int MeshLoader::getIndex(int index) const
{
    return indices_[index];
}
