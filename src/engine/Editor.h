#pragma once

#include "imgui.h"

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

    int LISTS_HEIGHT = 22;
    ImVec4 HIGHLIGHT_COLOR_NAMES{0.6, 0.6, 1, 1};
    ImVec4 HIGHLIGHT_COLOR_OTHER{1, 0.6, 0.6, 1};
};