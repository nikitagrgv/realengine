#include "Editor.h"

#include "EngineGlobals.h"
#include "MaterialManager.h"
#include "MeshManager.h"
#include "ShaderManager.h"
#include "TextureManager.h"

#include "glm/gtc/type_ptr.hpp"

Editor::Editor() {}

void Editor::render()
{
    render_main();
    render_materials();
    render_textures();
    render_shaders();
    render_meshes();
}

void Editor::render_main()
{
    ImGui::SetNextWindowSize(ImVec2(180, 120), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::Begin("Editor");

    constexpr int OFFSET = 110;

    ImGui::Checkbox("Materials", &materials_window_);
    ImGui::SameLine(OFFSET);
    ImGui::Checkbox("Shaders", &shaders_window_);

    ImGui::Checkbox("Textures", &texture_window_);
    ImGui::SameLine(OFFSET);
    ImGui::Checkbox("Meshes", &meshes_window_);
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
        const int num_mat = eng.material_manager->getCount();
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
                Shader *shader = material->getShader();
                if (shader)
                {
                    ImGui::AlignTextToFramePadding();
                    ImGui::TextColored(HIGHLIGHT_COLOR_NAMES, "%s",
                        eng.shader_manager->getName(shader));
                    ImGui::SameLine();
                    if (ImGui::Button("Go##shader"))
                    {
                        selected_shader_ = eng.shader_manager->getIndex(shader);
                        shaders_window_ = true;
                    }
                }
                else
                {
                    ImGui::TextDisabled("None");
                }
            }
            {
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

                    constexpr float SPEED = 0.1f;
                    constexpr const char *FORMAT = "%.3f";

                    ImGui::PushID(i);

                    switch (material->getParameterType(i))
                    {
                    case Material::ParameterType::Float:
                    {
                        float v = material->getParameterFloat(i);
                        if (ImGui::DragFloat("##", &v, SPEED, 0, 0, FORMAT))
                        {
                            material->setParameterFloat(i, v);
                        }
                        break;
                    }
                    case Material::ParameterType::Vec2:
                    {
                        glm::vec2 v = material->getParameterVec2(i);
                        if (ImGui::DragFloat2("##", glm::value_ptr(v), SPEED, 0, 0, FORMAT))
                        {
                            material->setParameterVec2(i, v);
                        }
                        break;
                    }
                    case Material::ParameterType::Vec3:
                    {
                        glm::vec3 v = material->getParameterVec3(i);
                        if (ImGui::ColorEdit3("##", glm::value_ptr(v), ImGuiColorEditFlags_Float))
                        {
                            material->setParameterVec3(i, v);
                        }
                        break;
                    }
                    case Material::ParameterType::Vec4:
                    {
                        glm::vec4 v = material->getParameterVec4(i);
                        if (ImGui::ColorEdit4("##", glm::value_ptr(v), ImGuiColorEditFlags_Float))
                        {
                            material->setParameterVec4(i, v);
                        }
                        break;
                    }
                    case Material::ParameterType::Mat4:
                    {
                        // TODO#
                        break;
                    }
                    default: break;
                    }

                    ImGui::PopID();

                    ImGui::Unindent();
                }
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

                        render_texture_info(texture);
                    }
                    else
                    {
                        ImGui::TextDisabled("Empty");
                    }

                    ImGui::Unindent();
                }
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

            Shader *shader = eng.shader_manager->get(selected_shader_);

            // TODO# SOURCE CODE VIEW
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

            // TODO# SOURCE CODE VIEW
        }
        ImGui::EndChild();
        ImGui::EndGroup();
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
