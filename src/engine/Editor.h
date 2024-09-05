#pragma once

#include "imgui.h"

#include "glm/mat4x4.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

class Texture;

class Editor
{
public:
    Editor();

    void render();

private:
    void render_main();

    void render_materials();
    void render_textures();
    void render_shaders();
    void render_meshes();

    void render_texture_info(Texture *texture);
    void render_texture(Texture *texture, float width, float height);

    static bool render_editor(float &v);
    static bool render_editor(glm::vec2 &v);
    static bool render_editor(glm::vec3 &v);
    static bool render_editor(glm::vec4 &v);
    static bool render_editor(glm::mat4 &v);

private:
    // Shaders
    int selected_shader_{0};
    bool shaders_window_{true};

    // Textures
    int selected_texture_{0};
    bool texture_window_{true};

    // Materials
    int selected_mat_{0};
    bool materials_window_{true};

    // Meshes
    int selected_mesh{0};
    bool meshes_window_{true};
};