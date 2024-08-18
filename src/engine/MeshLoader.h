#pragma once

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include <string>
#include <vector>

class MeshLoader
{
public:
    explicit MeshLoader(const char *path);
    ~MeshLoader() = default;

    bool isLoaded() const { return is_loaded_; }

    int getNumVertices() const;
    glm::vec3 getVertexPosition(int index) const;
    glm::vec2 getVertexTextureCoords(int index) const;
    glm::vec3 getVertexNormal(int index) const;

    int getNumIndices() const;
    unsigned int getIndex(int index) const;

private:
    bool is_loaded_{false};

    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 texture_coords;
        glm::vec3 normal;
    };

    std::vector<Vertex> vertices_;
    std::vector<unsigned int> indices_;
};