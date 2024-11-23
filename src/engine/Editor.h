#pragma once

#include "imgui.h"
#include "signals/Signals.h"

#include "glm/mat4x4.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include <string>
#include <utility>


class Material;
class NodeMesh;
class Node;
class Texture;

class Editor
{
public:
    Editor();
    ~Editor();

    Texture *getSelectedTexture() const;

    Node *getSelectedNode() const;
    void setSelectedNode(Node *node);

    void setPlayerPositionInfo(glm::vec3 pos) { player_pos_ = pos; }

    void addPopup(const char *message, float time_sec = 2.0f);

private:
    struct PopupInfo
    {
        PopupInfo() = default;
        PopupInfo(std::string message, float time_sec, uint64_t time_start_ms)
            : message(std::move(message))
            , duration_sec(time_sec)
            , create_time_ms(time_start_ms)
        {}

        std::string message;
        float duration_sec{};
        uint64_t create_time_ms{};
    };

private:
    void load_configs();
    void save_configs();

    void render();

    void render_main();

    void render_world();
    void render_materials();
    void render_settings();
    void render_textures();
    void render_shaders();
    void render_meshes();
    void render_info();
    void render_popup();

    void render_texture_info(Texture *texture);
    void render_texture(Texture *texture, float width, float height);

    void render_node(NodeMesh *node);

    void visualize_selected_node();

    void draw_tree(Material *mat);

    static bool render_editor(float &v);
    static bool render_editor(glm::vec2 &v);
    static bool render_editor(glm::vec3 &v);
    static bool render_editor(glm::vec4 &v);
    bool render_editor(glm::mat4 &v);

private:
    glm::vec3 player_pos_{};

    bool hide_all_{false};

    // World
    int selected_node_{0};
    bool nodes_window_{true};

    // Shaders
    int selected_shader_{0};
    bool shaders_window_{true};

    // Textures
    int selected_texture_{0};
    bool texture_window_{true};

    // Materials
    int selected_mat_{0};
    bool materials_window_{true};
    bool flat_mode_{false};

    // Meshes
    int selected_mesh{0};
    bool meshes_window_{true};

    // Info
    double fps_{0.0f};
    float last_update_fps_time_{0.0f};
    bool info_window_{true};

    // Settings
    bool settings_window_{true};

    struct Mat4WidgetData
    {
        glm::vec3 pos{};
        glm::vec3 scale{};
        glm::vec3 angles{};
        bool used = false;
        bool on_hold = false;
        bool initialized = false;
    };
    std::unordered_map<ImGuiID, Mat4WidgetData> widgets_data_;

    // Popups
    std::vector<PopupInfo> popups_;

    Context ctx_;
};