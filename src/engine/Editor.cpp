#include "Editor.h"

#include "EngineGlobals.h"
#include "Gui.h"
#include "MaterialManager.h"
#include "MeshManager.h"
#include "NodeMesh.h"
#include "Renderer.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "Visualizer.h"
#include "World.h"
#include "input/Input.h"
#include "time/Time.h"

#include "glm/gtc/type_ptr.hpp"

namespace
{

constexpr int LISTS_HEIGHT = 22;
constexpr ImVec4 HIGHLIGHT_COLOR_NAMES{0.6, 0.6, 1, 1};
constexpr ImVec4 HIGHLIGHT_COLOR_OTHER{1, 0.6, 0.6, 1};

constexpr float SPEED = 0.1f;
constexpr const char *FORMAT = "%.3f";

} // namespace

Editor::Editor()
{
    eng.gui->getSignalOnUpdate().connect(ctx_, [this] { render(); });
}

void Editor::render()
{
    if (eng.input->isKeyPressed(Key::KEY_F2))
    {
        hide_all_ = !hide_all_;
    }

    render_main();
    if (hide_all_)
    {
        return;
    }
    render_world();
    render_materials();
    render_textures();
    render_shaders();
    render_meshes();
    render_info();

    visualize_selected_node();
}

void Editor::render_main()
{
    ImGui::SetNextWindowSize(ImVec2(180, 120), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::Begin("Editor");

    constexpr int OFFSET = 110;

    ImGui::Checkbox("Hide All", &hide_all_);
    ImGui::SameLine(OFFSET);
    ImGui::Checkbox("Info", &info_window_);

    ImGui::Checkbox("Materials", &materials_window_);
    ImGui::SameLine(OFFSET);
    ImGui::Checkbox("Shader Sources", &shaders_window_);

    ImGui::Checkbox("Textures", &texture_window_);
    ImGui::SameLine(OFFSET);
    ImGui::Checkbox("Meshes", &meshes_window_);

    ImGui::Checkbox("World", &nodes_window_);

    ImGui::End();
}

void Editor::render_world()
{
    if (!nodes_window_)
    {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(380, 340), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(0, 550), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Nodes", &nodes_window_))
    {
        ImGui::End();
        return;
    }

    // Left
    {
        ImGui::BeginChild("left pane", ImVec2(150, 0),
            ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
        const int num_nodes = eng.world->getNumNodes();
        for (int i = 0; i < num_nodes; i++)
        {
            ImGui::PushID(i);

            Node *node = eng.world->getNodeByIndex(i);

            bool enabled = node->isEnabled();
            if (ImGui::Checkbox("##", &enabled))
            {
                node->setEnabled(enabled);
            }

            ImGui::SameLine();

            const char *name = node->getName().c_str();
            char label[64];
            sprintf(label, "%d. %.50s", i, name);
            if (ImGui::Selectable(label, selected_node_ == i, 0, ImVec2(0, LISTS_HEIGHT)))
            {
                selected_node_ = i;
            }

            ImGui::PopID();
        }
        ImGui::EndChild();
    }
    ImGui::SameLine();

    // Right
    {
        ImGui::BeginGroup();
        ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
        if (eng.world->hasNodeIndex(selected_node_))
        {
            Node *node = eng.world->getNodeByIndex(selected_node_);

            ImGui::TextColored(HIGHLIGHT_COLOR_NAMES, "%s", node->getName().c_str());
            ImGui::SameLine();
            ImGui::TextColored(HIGHLIGHT_COLOR_OTHER, "(%s)", node->getTypeName());

            ImGui::Text("ID:");
            ImGui::SameLine();
            ImGui::TextColored(HIGHLIGHT_COLOR_OTHER, "%d", node->getId());

            ImGui::Separator();

            bool enabled = node->isEnabled();
            if (ImGui::Checkbox("Enabled", &enabled))
            {
                node->setEnabled(enabled);
            }

            {
                ImGui::SeparatorText("Transform");
                glm::mat4 tr = node->getTransform();

                ImGui::PushItemWidth(-FLT_MIN);
                if (render_editor(tr))
                {
                    node->setTransform(tr);
                }
                ImGui::PopItemWidth();
            }

            if (auto n = node->cast<NodeMesh>())
            {
                render_node(n);
            }
        }
        ImGui::EndChild();
        ImGui::EndGroup();
    }

    ImGui::End();
}

void Editor::render_materials()
{
    if (!materials_window_)
    {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(380, 340), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(0, 200), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Materials", &materials_window_))
    {
        ImGui::End();
        return;
    }

    // Left
    {
        ImGui::BeginChild("left pane", ImVec2(150, 0),
            ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);

        ImGui::Checkbox("Flat", &flat_mode_);

        ImGui::Separator();

        const int num_mat = eng.material_manager->getCount();
        if (flat_mode_)
        {
            for (int i = 0; i < num_mat; i++)
            {
                const char *name = eng.material_manager->getName(i);
                char label[64];
                sprintf(label, "%d. %.50s", i, name);
                if (ImGui::Selectable(label, selected_mat_ == i, 0, ImVec2(0, LISTS_HEIGHT)))
                {
                    selected_mat_ = i;
                }
            }
        }
        else
        {
            for (int i = 0; i < num_mat; i++)
            {
                Material *mat = eng.material_manager->get(i);
                if (!mat->isBase())
                {
                    continue;
                }

                if (ImGui::BeginTable("##tree", 1, ImGuiTableFlags_RowBg))
                {
                    draw_tree(mat);
                    ImGui::EndTable();
                }
            }
        }

        ImGui::EndChild();
    }
    ImGui::SameLine();

    // Right
    {
        ImGui::BeginGroup();
        ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
        if (eng.material_manager->contains(selected_mat_))
        {
            ImGui::TextColored(HIGHLIGHT_COLOR_NAMES, "%s",
                eng.material_manager->getName(selected_mat_));

            ImGui::Separator();

            Material *material = eng.material_manager->get(selected_mat_);

            {
                ImGui::SeparatorText("Shader");
                ShaderSource *source = material->getShaderSource();
                if (source)
                {
                    ImGui::AlignTextToFramePadding();
                    ImGui::TextColored(HIGHLIGHT_COLOR_NAMES, "%s",
                        eng.shader_manager->getName(source));
                    ImGui::SameLine();
                    if (ImGui::Button("Go##shader"))
                    {
                        selected_shader_ = eng.shader_manager->getIndex(source);
                        shaders_window_ = true;
                    }
                }
                else
                {
                    ImGui::TextDisabled("None");
                }
            }

            {
                ImGui::SeparatorText("Options");

                bool two_sided = material->isTwoSided();
                if (ImGui::Checkbox("Two Sided", &two_sided))
                {
                    material->setTwoSided(two_sided);
                }
            }

            {
                ImGui::PushID("params");
                ImGui::SeparatorText("Parameters");
                const int num_params = material->getNumParameters();
                for (int i = 0; i < num_params; ++i)
                {
                    ImGui::Bullet();
                    ImGui::TextColored(HIGHLIGHT_COLOR_NAMES, "%s",
                        material->getParameterName(i).c_str());
                    ImGui::SameLine();
                    ImGui::TextColored(HIGHLIGHT_COLOR_OTHER, "(%s)",
                        material->getParameterTypeName(i));

                    ImGui::Indent();

                    ImGui::PushID(i);

                    switch (material->getParameterType(i))
                    {
                    case Material::ParameterType::Float:
                    {
                        float v = material->getParameterFloat(i);
                        if (render_editor(v))
                        {
                            material->setParameterFloat(i, v);
                        }
                        break;
                    }
                    case Material::ParameterType::Vec2:
                    {
                        glm::vec2 v = material->getParameterVec2(i);
                        if (render_editor(v))
                        {
                            material->setParameterVec2(i, v);
                        }
                        break;
                    }
                    case Material::ParameterType::Vec3:
                    {
                        glm::vec3 v = material->getParameterVec3(i);
                        if (render_editor(v))
                        {
                            material->setParameterVec3(i, v);
                        }
                        break;
                    }
                    case Material::ParameterType::Vec4:
                    {
                        glm::vec4 v = material->getParameterVec4(i);
                        if (render_editor(v))
                        {
                            material->setParameterVec4(i, v);
                        }
                        break;
                    }
                    case Material::ParameterType::Mat4:
                    {
                        glm::mat4 v = material->getParameterMat4(i);
                        if (render_editor(v))
                        {
                            material->setParameterMat4(i, v);
                        }
                        break;
                    }
                    default: break;
                    }

                    ImGui::PopID();

                    ImGui::Unindent();
                }
                ImGui::PopID();
            }

            {
                ImGui::SeparatorText("Textures");
                const int num_textures = material->getNumTextures();
                for (int i = 0; i < num_textures; ++i)
                {
                    ImGui::Bullet();
                    ImGui::TextColored(HIGHLIGHT_COLOR_NAMES, "%s",
                        material->getTextureName(i).c_str());

                    Texture *texture = material->getTexture(i);

                    ImGui::Indent();

                    if (texture)
                    {
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("Name:");

                        ImGui::SameLine();
                        ImGui::AlignTextToFramePadding();
                        ImGui::TextColored(HIGHLIGHT_COLOR_NAMES, "%s",
                            eng.texture_manager->getName(texture));

                        ImGui::SameLine();
                        if (ImGui::Button("Go##texture"))
                        {
                            texture_window_ = true;
                            selected_texture_ = eng.texture_manager->getIndex(texture);
                        }

                        if (ImGui::BeginPopupContextItem())
                        {
                            for (int j = 0; j < eng.texture_manager->getCount(); ++j)
                            {
                                const char *name = eng.texture_manager->getName(j);
                                if (ImGui::Button(name))
                                {
                                    material->setTexture(i, eng.texture_manager->get(j));
                                    ImGui::CloseCurrentPopup();
                                }
                            }

                            ImGui::EndPopup();
                        }

                        render_texture_info(texture);
                    }
                    else
                    {
                        ImGui::TextDisabled("Empty");
                    }

                    ImGui::Unindent();
                }
            }

            {
                ImGui::PushID("defines");
                ImGui::SeparatorText("Defines");
                const int num_defines = material->getNumDefines();
                for (int i = 0; i < num_defines; ++i)
                {
                    ImGui::Bullet();

                    bool enabled = material->getDefine(i);
                    ImGui::PushID(i);
                    if (ImGui::Checkbox("##", &enabled))
                    {
                        material->setDefine(i, enabled);
                    }
                    ImGui::PopID();

                    ImGui::SameLine();

                    ImGui::TextColored(HIGHLIGHT_COLOR_NAMES, "%s",
                        material->getDefineName(i).c_str());
                }
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
        ImGui::EndGroup();
    }

    ImGui::End();
}

void Editor::render_textures()
{
    if (!texture_window_)
    {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(380, 340), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(0, 550), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Textures", &texture_window_))
    {
        ImGui::End();
        return;
    }

    // Left
    {
        ImGui::BeginChild("left pane", ImVec2(150, 0),
            ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
        const int num_mat = eng.texture_manager->getCount();
        for (int i = 0; i < num_mat; i++)
        {
            render_texture(eng.texture_manager->get(i), LISTS_HEIGHT - 2, LISTS_HEIGHT - 2);

            ImGui::SameLine();

            const char *name = eng.texture_manager->getName(i);
            char label[64];
            sprintf(label, "%d. %.50s", i, name);
            if (ImGui::Selectable(label, selected_texture_ == i, 0, ImVec2(0, LISTS_HEIGHT)))
            {
                selected_texture_ = i;
            }
        }
        ImGui::EndChild();
    }
    ImGui::SameLine();

    // Right
    {
        ImGui::BeginGroup();
        ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
        if (eng.texture_manager->contains(selected_texture_))
        {
            ImGui::TextColored(HIGHLIGHT_COLOR_NAMES, "%s",
                eng.texture_manager->getName(selected_texture_));

            ImGui::Separator();

            Texture *texture = eng.texture_manager->get(selected_texture_);
            render_texture_info(texture);
        }
        ImGui::EndChild();
        ImGui::EndGroup();
    }

    ImGui::End();
}

void Editor::render_shaders()
{
    if (!shaders_window_)
    {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(380, 340), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(0, 550), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Shaders", &shaders_window_))
    {
        ImGui::End();
        return;
    }

    // Left
    {
        ImGui::BeginChild("left pane", ImVec2(150, 0),
            ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
        const int num_shaders = eng.shader_manager->getCount();
        for (int i = 0; i < num_shaders; i++)
        {
            const char *name = eng.shader_manager->getName(i);
            char label[64];
            sprintf(label, "%d. %.50s", i, name);
            if (ImGui::Selectable(label, selected_shader_ == i, 0, ImVec2(0, LISTS_HEIGHT)))
            {
                selected_shader_ = i;
            }
        }
        ImGui::EndChild();
    }
    ImGui::SameLine();

    // Right
    {
        ImGui::BeginGroup();
        ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
        if (eng.shader_manager->contains(selected_shader_))
        {
            ImGui::TextColored(HIGHLIGHT_COLOR_NAMES, "%s",
                eng.shader_manager->getName(selected_shader_));

            ImGui::Separator();

            ShaderSource *source = eng.shader_manager->get(selected_shader_);
            const std::string file = source->getFilepath();

            ImGui::Text("File:");
            ImGui::SameLine();
            if (file.empty())
            {
                ImGui::TextDisabled("None");
            }
            else
            {
                ImGui::TextColored(HIGHLIGHT_COLOR_NAMES, "%s", file.c_str());
            }

            // TODO: source code view
        }
        ImGui::EndChild();
        ImGui::EndGroup();
    }

    ImGui::End();
}

void Editor::render_meshes()
{
    if (!meshes_window_)
    {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(380, 340), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(0, 550), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Meshes", &meshes_window_))
    {
        ImGui::End();
        return;
    }

    // Left
    {
        ImGui::BeginChild("left pane", ImVec2(150, 0),
            ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
        const int num_meshes = eng.mesh_manager->getCount();
        for (int i = 0; i < num_meshes; i++)
        {
            const char *name = eng.mesh_manager->getName(i);
            char label[64];
            sprintf(label, "%d. %.50s", i, name);
            if (ImGui::Selectable(label, selected_mesh == i, 0, ImVec2(0, LISTS_HEIGHT)))
            {
                selected_mesh = i;
            }
        }
        ImGui::EndChild();
    }
    ImGui::SameLine();

    // Right
    {
        ImGui::BeginGroup();
        ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
        if (eng.mesh_manager->contains(selected_mesh))
        {
            ImGui::TextColored(HIGHLIGHT_COLOR_NAMES, "%s",
                eng.mesh_manager->getName(selected_mesh));

            ImGui::Separator();

            Mesh *mesh = eng.mesh_manager->get(selected_mesh);

            ImGui::Text("Vertices:");
            ImGui::SameLine();
            ImGui::TextColored(HIGHLIGHT_COLOR_OTHER, "%d", mesh->getNumVertices());

            ImGui::Text("Indices:");
            ImGui::SameLine();
            ImGui::TextColored(HIGHLIGHT_COLOR_OTHER, "%d", mesh->getNumIndices());
        }
        ImGui::EndChild();
        ImGui::EndGroup();
    }

    ImGui::End();
}

void Editor::render_info()
{
    if (!info_window_)
    {
        return;
    }

    if (last_update_fps_time_ < eng.time->getTime() - 0.5f)
    {
        last_update_fps_time_ = eng.time->getTime();
        fps_ = eng.time->getFps();
    }

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    {
        const float PAD = 10.0f;
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = work_pos.x + work_size.x - PAD;
        window_pos.y = work_pos.y + PAD;
        window_pos_pivot.x = 1.0f;
        window_pos_pivot.y = 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        window_flags |= ImGuiWindowFlags_NoMove;
    }
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin("Info", &info_window_, window_flags))
    {
        const glm::vec2 mpos = eng.input->getMousePos();

        ImGui::Text("Frame: %d", eng.time->getFrame());
        ImGui::Text("FPS: %.1f", fps_);
        ImGui::Text("Mouse: %5.0f,%5.0f", mpos.x, mpos.y);
        ImGui::SeparatorText("Frame");
        ImGui::Text("Rendered Indices: %llu", eng.stat.getNumRenderedIndicesInFrame());
        ImGui::Text("Compiled Shaders: %llu", eng.stat.getNumCompiledShadersInFrame());
        ImGui::SeparatorText("Total");
        ImGui::Text("Rendered Indices: %llu", eng.stat.getNumRenderedIndicesTotal());
        ImGui::Text("Compiled Shaders: %llu", eng.stat.getNumCompiledShadersTotal());
        ImGui::Separator();
    }
    ImGui::End();
}

void Editor::render_texture_info(Texture *texture)
{
    if (!texture)
    {
        ImGui::TextDisabled("Empty");
        return;
    }

    if (!texture->isLoaded())
    {
        ImGui::TextDisabled("Not loaded");
        return;
    }

    const int orig_width = texture->getWidth();
    const int orig_height = texture->getHeight();

    ImGui::Text("Size:");
    ImGui::SameLine();
    ImGui::TextColored(HIGHLIGHT_COLOR_OTHER, "%dx%d", orig_width, orig_height);

    float preview_width = orig_width;
    float preview_height = orig_height;
    constexpr float MAX_SIZE = 128.0f;
    bool resized = false;
    while (preview_width > MAX_SIZE || preview_height > MAX_SIZE)
    {
        preview_width *= 0.5;
        preview_height *= 0.5;
        resized = true;
    }

    render_texture(texture, preview_width, preview_height);
    if (resized)
    {
        ImGui::SameLine();
        ImGui::TextDisabled("(preview %dx%d)", (int)preview_width, (int)preview_height);
    }
}

void Editor::render_texture(Texture *texture, float width, float height)
{
    if (texture && texture->isLoaded())
    {
        ImGui::Image(texture->getID(), ImVec2(width, height));
    }
    else
    {
        ImGui::Dummy(ImVec2(width, height));
    }
}

void Editor::render_node(NodeMesh *node)
{
    ImGui::SeparatorText("NodeMesh");

    Mesh *mesh = node->getMesh();
    Material *material = node->getMaterial();

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Mesh:");
    ImGui::SameLine();
    if (mesh)
    {
        ImGui::TextColored(HIGHLIGHT_COLOR_NAMES, "%s", eng.mesh_manager->getName(mesh));
        ImGui::SameLine();
        if (ImGui::Button("Go##mesh"))
        {
            selected_mesh = eng.mesh_manager->getIndex(mesh);
            meshes_window_ = true;
        }
    }
    else
    {
        ImGui::TextDisabled("None");
    }

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Material:");
    ImGui::SameLine();
    if (material)
    {
        ImGui::TextColored(HIGHLIGHT_COLOR_NAMES, "%s", eng.material_manager->getName(material));
        ImGui::SameLine();
        if (ImGui::Button("Go##mat"))
        {
            selected_mat_ = eng.material_manager->getIndex(material);
            materials_window_ = true;
        }
    }
    else
    {
        ImGui::TextDisabled("None");
    }
}

void Editor::visualize_selected_node()
{
    if (!eng.world->hasNodeIndex(selected_node_))
    {
        return;
    }
    Node *n = eng.world->getNodeByIndex(selected_node_);
    const math::BoundBox &bb = n->getGlobalBoundBox();
    eng.visualizer->addBoundBox(bb, glm::vec4{1.0f, 0.0f, 0.0f, 1.0f}, false);
}

void Editor::draw_tree(Material *mat)
{
    // const char *name = eng.material_manager->getName(i);
    // char label[64];
    // sprintf(label, "%d. %.50s", i, name);
    // if (ImGui::Selectable(label, selected_mat_ == i, 0, ImVec2(0, LISTS_HEIGHT)))
    // {
    //     selected_mat_ = i;
    // }
    const int num_children = mat->getNumChildren();
    const bool leaf = num_children == 0;
    const int mat_index = eng.material_manager->getIndex(mat);
    const char *mat_name = eng.material_manager->getName(mat);
    assert(mat_index != -1);

    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::PushID(mat_index);
    ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_None;
    tree_flags |= ImGuiTreeNodeFlags_DefaultOpen;
    tree_flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    tree_flags |= ImGuiTreeNodeFlags_NavLeftJumpsBackHere;
    if (selected_mat_ == mat_index)
    {
        tree_flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (leaf)
    {
        tree_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
    }
    const bool node_open = ImGui::TreeNodeEx("", tree_flags, "%s", mat_name);
    if (ImGui::IsItemFocused())
    {
        selected_mat_ = mat_index;
    }
    if (node_open)
    {
        for (int j = 0, count = mat->getNumChildren(); j < count; ++j)
        {
            Material *child = mat->getChild(j);
            draw_tree(child);
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

bool Editor::render_editor(float &v)
{
    return ImGui::DragFloat("##", &v, SPEED, 0, 0, FORMAT);
}

bool Editor::render_editor(glm::vec2 &v)
{
    return ImGui::DragFloat2("##", glm::value_ptr(v), SPEED, 0, 0, FORMAT);
}

bool Editor::render_editor(glm::vec3 &v)
{
    return ImGui::ColorEdit3("##", glm::value_ptr(v), ImGuiColorEditFlags_Float);
}

bool Editor::render_editor(glm::vec4 &v)
{
    return ImGui::ColorEdit4("##", glm::value_ptr(v), ImGuiColorEditFlags_Float);
}

bool Editor::render_editor(glm::mat4 &v)
{
    bool changed = false;
    float *ptr = glm::value_ptr(v);
    constexpr int STRIDE = 4;

    for (int i = 0; i < 4; ++i)
    {
        ImGui::PushID(i);
        changed |= ImGui::DragFloat4("##", ptr + STRIDE * i, SPEED, 0, 0, FORMAT);
        ImGui::PopID();
    }
    return changed;
}
