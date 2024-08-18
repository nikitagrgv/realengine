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
    struct ObjVertex
    {
        int position_index = -1;
        int texture_coords_index = -1;
        int normals_index = -1;

        bool operator==(const ObjVertex &rhs) const
        {
            return position_index == rhs.position_index
                && texture_coords_index == rhs.texture_coords_index
                && normals_index == rhs.normals_index;
        }
        bool operator!=(const ObjVertex &rhs) const { return !(rhs == *this); }
    };

    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 texture_coords;
        glm::vec3 normal;
    };

    struct Face
    {
        unsigned int vertex_indices[3]{0, 0, 0};
    };

private:
    void handle_name_token(std::stringstream &line);
    void handle_position_token(std::stringstream &line);
    void handle_texture_coords_token(std::stringstream &line);
    void handle_normal_token(std::stringstream &line);
    void handle_surface_token(std::stringstream &line);
    void handle_face_token(std::stringstream &line);

    unsigned int create_or_get_vertex_index(const ObjVertex &obj_vertex);

private:
    bool is_loaded_{false};

    std::string name_{"Noname"};

    std::vector<glm::vec3> positions_;
    std::vector<glm::vec2> texture_coords_;
    std::vector<glm::vec3> normals_;
    std::vector<ObjVertex> obj_vertices_;
    std::vector<Face> faces_;

    std::vector<Vertex> vertices_;
    std::vector<unsigned int> indices_;
};