// clang-format off
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "Camera.h"
#include "EngineGlobals.h"
#include "Image.h"
#include "Material.h"
#include "MaterialManager.h"
#include "Mesh.h"
#include "MeshLoader.h"
#include "MeshManager.h"
#include "Shader.h"
#include "ShaderManager.h"
#include "Texture.h"
#include "TextureManager.h"
#include "VertexArrayObject.h"
#include "VertexBufferObject.h"
#include "Visualizer.h"
#include "fs/FileSystem.h"
#include "time/Time.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/mat4x4.hpp"
#include <iostream>
#include <vector>

const unsigned int DEFAULT_WIDTH = 1;
const unsigned int DEFAULT_HEIGHT = 1;

class Engine
{
public:
    void exec()
    {
        init();
        glfwMaximizeWindow(window_);


        ///////////////////////////////////////////////////////////////////////////////
        Shader *light_cube_shader = eg.shader_manager->create("light_cube");
        light_cube_shader->loadFile("light_cube.shader");

        Mesh *light_cube_mesh = eg.mesh_manager->create();
        light_cube_mesh->addVertex({0.0f, 1.0f, 0.0f});
        light_cube_mesh->addVertex({1.0f, 0.0f, 0.0f});
        light_cube_mesh->addVertex({-1.0f, 0.0f, 0.0f});
        light_cube_mesh->addVertex({0.0f, 0.0f, 1.0f});
        light_cube_mesh->addIndices(0, 1, 2);
        light_cube_mesh->addIndices(0, 1, 3);
        light_cube_mesh->addIndices(0, 2, 3);
        light_cube_mesh->addIndices(1, 3, 2);
        light_cube_mesh->flush();

        ///////////////////////////////////////////////////////////////////////////////
        Shader *shader = eg.shader_manager->create("basic");
        shader->loadFile("shader.shader");

        ///////////////////////////////////////////////////////////////////////////////
        Texture *cat_texture = eg.texture_manager->create();
        cat_texture->load("image.png");
        glm::mat4 cat_transform = glm::mat4{1.0f};
        Mesh *cat_mesh = eg.mesh_manager->create("cat");
        MeshLoader::loadToMesh("object.obj", cat_mesh);

        Material *cat_material = eg.material_manager->create("cat");
        cat_material->setShader(shader);
        cat_material->addTexture("uTexture");
        cat_material->setTexture("uTexture", cat_texture);
        cat_material->addParameterFloat("uMaterial.shininess");
        cat_material->setParameterFloat("uMaterial.shininess", 32.0f);

        ////////////////////////////////////////////////
        Texture *stickman_texture = eg.texture_manager->create();
        stickman_texture->load("image2.png");

        glm::mat4 stickman_transform = glm::mat4{1.0f};
        Mesh *stickman_mesh = eg.mesh_manager->create("stickman");
        MeshLoader::loadToMesh("stickman.obj", stickman_mesh, true);

        ////////////////////////////////////////////////
        Texture *floor_texture = eg.texture_manager->create();
        floor_texture->load("floor.png");

        Mesh *floor_mesh = eg.mesh_manager->create();
        const float floor_size = 10.0f;
        const float floor_y = 0.0f;
        const float max_text_coord = 10.0f;
        floor_mesh->addVertex({-floor_size, floor_y, -floor_size}, {0, 1, 0}, {0.0f, 0.0f});
        floor_mesh->addVertex({-floor_size, floor_y, floor_size}, {0, 1, 0},
            {0.0f, max_text_coord});
        floor_mesh->addVertex({floor_size, floor_y, floor_size}, {0, 1, 0},
            {max_text_coord, max_text_coord});
        floor_mesh->addVertex({floor_size, floor_y, -floor_size}, {0, 1, 0},
            {max_text_coord, 0.0f});
        floor_mesh->addIndices(0, 1, 3);
        floor_mesh->addIndices(1, 2, 3);
        floor_mesh->flush();
        ////////////////////////////////////////////////

        camera_.setTransform(glm::translate(glm::mat4{1.0f}, glm::vec3(0.0f, 0.0f, 3.0f)));
        update_proj(window_);

        const auto visualize_normals = [](const Mesh *mesh, const glm::mat4 &transform) {
            return;
            const auto to_local = [&](const glm::vec3 &v) {
                return transform * glm::vec4(v, 1);
            };
            for (int i = 0; i < mesh->getNumVertices(); i++)
            {
                const auto pos = mesh->getVertexPos(i);
                const auto norm = mesh->getVertexNormal(i);
                eg.visualizer->addLine(to_local(pos), to_local(pos + norm),
                    {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.2f});
            }
        };

        float anim_time = 0.0f;
        glm::vec3 light_pos{1, 1, 1};

        float anim_time_multiplier = 1.0f;
        glm::vec3 light_color{0.2, 0.65, 0.65};
        float ambient_power = 0.1f;
        float diffuse_power = 1.0f;
        float specular_power = 1.0f;
        float shininess = 32.0f;

        bool use_ambient = true;
        bool use_diffuse = true;
        bool use_specular = true;

        const auto use_material = [](Material *material) {
            Shader *shader = material->getShader();
            shader->bind();
            for (int i = 0, count = material->getNumTextures(); i < count; i++)
            {
                const int loc = shader->getUniformLocation(material->getTextureName(i).c_str());
                if (loc == -1)
                {
                    continue;
                }
                material->getTexture(i)->bind(i);
                shader->setUniformInt(loc, i);
            }
            for (int i = 0, count = material->getNumParameters(); i < count; i++)
            {
                const int loc = shader->getUniformLocation(material->getParameterName(i).c_str());
                if (loc == -1)
                {
                    continue;
                }
                const auto type = material->getParameterType(i);
                switch (type)
                {
                case Material::ParameterType::Float:
                    shader->setUniformFloat(loc, material->getParameterFloat(i));
                    break;
                case Material::ParameterType::Vec2:
                    shader->setUniformVec2(loc, material->getParameterVec2(i));
                    break;
                case Material::ParameterType::Vec3:
                    shader->setUniformVec3(loc, material->getParameterVec3(i));
                    break;
                case Material::ParameterType::Vec4:
                    shader->setUniformVec4(loc, material->getParameterVec4(i));
                    break;
                case Material::ParameterType::Mat4:
                    shader->setUniformMat4(loc, material->getParameterMat4(i));
                    break;
                default: break;
                }
            }
        };

        class Editor
        {
        public:
            Editor() {}

            void render()
            {
                render_main();
                render_materials();
            }

        private:
            void render_main()
            {
                ImGui::SetNextWindowSize(ImVec2(180, 120), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
                ImGui::Begin("Editor");
                ImGui::Checkbox("Materials", &materials_window_);
                ImGui::End();
            }

            void render_materials()
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
                    const int num_mat = eg.material_manager->getCount();
                    for (int i = 0; i < num_mat; i++)
                    {
                        const char *name = eg.material_manager->getName(i);
                        char label[64];
                        sprintf(label, "%d. %.50s", i, name);
                        if (ImGui::Selectable(label, selected_mat_ == i))
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
                    if (eg.material_manager->contains(selected_mat_))
                    {
                        ImGui::Text("%s", eg.material_manager->getName(selected_mat_));

                        ImGui::Separator();

                        Material *material = eg.material_manager->get(selected_mat_);

                        {
                            ImGui::SeparatorText("Shader");
                            Shader *shader = material->getShader();
                            if (shader)
                            {
                                char name[64];
                                sprintf(name, "%.50s", eg.shader_manager->getName(shader));
                                if (ImGui::Button(name))
                                {
                                    // TODO open shader
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
                                ImGui::BulletText("%s", material->getParameterName(i).c_str());
                                ImGui::SameLine();
                                ImGui::TextColored(ImVec4(1, 0.6, 0.6, 1), "(%s)",
                                    material->getParameterTypeName(i));

                                ImGui::Indent();

                                switch (material->getParameterType(i))
                                {
                                case Material::ParameterType::Float:
                                {
                                    float v = material->getParameterFloat(i);
                                    if (ImGui::DragFloat("##", &v, 0.1f))
                                    {
                                        material->setParameterFloat(i, v);
                                    }
                                    break;
                                }
                                case Material::ParameterType::Vec2:
                                {
                                    break;
                                }
                                case Material::ParameterType::Vec3:
                                {
                                    break;
                                }
                                case Material::ParameterType::Vec4:
                                {
                                    break;
                                }
                                case Material::ParameterType::Mat4:
                                {
                                    break;
                                }
                                default: break;
                                }

                                ImGui::Unindent();
                                // material->getParameterName()
                            }
                        }

                        {
                            ImGui::SeparatorText("Textures");
                        }

                        ImGui::Text("AFAFFAF");
                    }
                    ImGui::EndChild();
                    ImGui::EndGroup();
                }

                ImGui::End();
            }

        private:
            // Shaders
            int selected_shader_{0};
            bool shaders_window_{true};

            // Materials
            int selected_mat_{0};
            bool materials_window_{true};
        };
        Editor editor;

        while (!exit_)
        {
            shader->setDefine("USE_AMBIENT", use_ambient);
            shader->setDefine("USE_DIFFUSE", use_diffuse);
            shader->setDefine("USE_SPECULAR", use_specular);

            if (shader->isDirty())
            {
                shader->recompile();
            }

            glfwPollEvents();

            //////////////////////////////////////////////// IMGUI
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            {
                ImGui::Begin("Parameters");
                ImGui::SliderFloat("Time multiplier", &anim_time_multiplier, 0.0f, 10.0f);
                ImGui::ColorEdit3("Light color", glm::value_ptr(light_color));
                ImGui::SliderFloat("Ambient power", &ambient_power, 0.0f, 1.0f);
                ImGui::SliderFloat("Diffuse power", &diffuse_power, 0.0f, 1.0f);
                ImGui::SliderFloat("Specular power", &specular_power, 0.0f, 1.0f);
                ImGui::SliderFloat("Shininess", &shininess, 0.0f, 64.0f);
                ImGui::Checkbox("Use ambient", &use_ambient);
                ImGui::Checkbox("Use diffuse", &use_diffuse);
                ImGui::Checkbox("Use specular", &use_specular);
                if (ImGui::Button("Button"))
                {}
                ImGui::SameLine();
                ImGui::Text("counter");

                ImGui::End();
            }

            ImGui::ShowDemoWindow(nullptr);

            editor.render();

            ImGui::Render();
            //////////////////////////////////////////////// IMGUI

            eg.time->update();

            process_input();

            if (glfwGetKey(window_, GLFW_KEY_F5) == GLFW_PRESS)
            {
                shader->recompile();
                light_cube_shader->recompile();
            }

            const auto add_axis = [](const glm::vec3 &axis) {
                eg.visualizer->addLine(glm::vec3{0, 0, 0}, axis, glm::vec4{axis, 1.0f});
            };
            add_axis(glm::vec3{1, 0, 0});
            add_axis(glm::vec3{0, 1, 0});
            add_axis(glm::vec3{0, 0, 1});

            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            anim_time += eg.time->getDelta() * anim_time_multiplier;
            light_pos.x = sin(6 + anim_time * 1.0351) * 1.5f + 0.5f;
            light_pos.y = cos(1 + anim_time * 1.2561) * 1.5f + 1.3f;
            light_pos.z = sin(7 + anim_time * 1.125) * 1.5f + 0.5f;

            shader->bind();
            shader->setUniformVec3("uLight.color", light_color);
            shader->setUniformVec3("uLight.pos", light_pos);
            shader->setUniformMat4("uViewProj", camera_.getViewProj());
            if (use_ambient)
            {
                shader->setUniformFloat("uLight.ambientPower", ambient_power);
            }
            if (use_diffuse)
            {
                shader->setUniformFloat("uLight.diffusePower", diffuse_power);
            }
            if (use_specular)
            {
                shader->setUniformVec3("uCameraPos", camera_.getPosition());
                shader->setUniformFloat("uLight.specularPower", specular_power);
                shader->setUniformFloat("uMaterial.shininess", shininess);
            }

            glCullFace(GL_BACK);

            cat_transform = glm::rotate(glm::mat4{1.0f}, float(0.25 * eg.time->getTime()),
                                glm::vec3(1.0f, 0.0f, 0.0f))
                * glm::scale(glm::mat4{1.0f}, glm::vec3{0.5f});
            shader->setUniformMat4("uModel", cat_transform);
            cat_texture->bind();
            cat_mesh->bind();
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            use_material(cat_material);
            glDrawElements(GL_TRIANGLES, cat_mesh->getNumIndices(), GL_UNSIGNED_INT, 0);


            ////////////////////////////////////////////////
            stickman_texture->bind();
            stickman_mesh->bind();
            stickman_mesh->flush();
            stickman_transform = glm::translate(glm::mat4{1.0f}, glm::vec3{1, 1, 0})
                * glm::scale(glm::mat4{1.0f}, glm::vec3{0.016f});
            shader->setUniformMat4("uModel", stickman_transform);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glDrawElements(GL_TRIANGLES, stickman_mesh->getNumIndices(), GL_UNSIGNED_INT, 0);

            ////////////////////////////////////////////////
            floor_texture->bind();
            floor_mesh->bind();
            floor_mesh->flush();
            shader->setUniformMat4("uModel", glm::translate(glm::mat4{1.0f}, glm::vec3{0, -1, 0}));
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glDrawElements(GL_TRIANGLES, floor_mesh->getNumIndices(), GL_UNSIGNED_INT, 0);

            ////////////////////////////////////////////////
            light_cube_shader->bind();
            light_cube_shader->setUniformVec3("uColor", light_color);
            light_cube_shader->setUniformMat4("uMVP",
                camera_.getMVP(glm::translate(glm::mat4{1.0f}, light_pos)
                    * glm::scale(glm::mat4{1.0f}, glm::vec3{0.08f})));
            light_cube_mesh->bind();
            light_cube_mesh->flush();
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDrawElements(GL_TRIANGLES, light_cube_mesh->getNumIndices(), GL_UNSIGNED_INT, 0);

            ///////////////////////////////////////////////
            visualize_normals(cat_mesh, cat_transform);
            visualize_normals(stickman_mesh, stickman_transform);
            visualize_normals(floor_mesh, glm::translate(glm::mat4{1.0f}, glm::vec3{0, -1, 0}));
            ////////////////////////////////////////////////
            eg.visualizer->render(camera_.getViewProj());


            ////////////////// IMGUI
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            ////////////////// IMGUI

            glfwSwapBuffers(window_);

            if (glfwWindowShouldClose(window_))
            {
                exit_ = true;
            }

            if (last_update_fps_time_ < eg.time->getTime() - 1.0f)
            {
                last_update_fps_time_ = eg.time->getTime();
                glfwSetWindowTitle(window_,
                    std::string("FPS: " + std::to_string(eg.time->getFps())).c_str());
            }
        }
    }

private:
    void init()
    {
        eg.engine_ = this;
        eg.time = new Time();
        eg.fs = new FileSystem();
        eg.texture_manager = new TextureManager();
        eg.shader_manager = new ShaderManager();
        eg.mesh_manager = new MeshManager();
        eg.material_manager = new MaterialManager();

        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        // glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // TODO DEBUG

        glfwSetErrorCallback([](int error, const char *description) {
            std::cout << "GLFW Error: " << description << std::endl;
        });

        window_ = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "LearnOpenGL", NULL, NULL);
        if (window_ == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
        }
        glfwMakeContextCurrent(window_);
        glfwSetFramebufferSizeCallback(window_, [](GLFWwindow *window, int width, int height) {
            eg.engine_->framebuffer_size_callback(window, width, height);
        });

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
        }

        //////////////////////////////////////
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window_, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        //////////////////////////////////////


        update_mouse();
        mouse_delta_x_ = 0;
        mouse_delta_y_ = 0;

        eg.visualizer = new Visualizer();
    }

    void shutdown()
    {
        const auto delete_and_null = [](auto &ptr) {
            assert(ptr);
            delete ptr;
            ptr = nullptr;
        };
        delete_and_null(eg.visualizer);
        delete_and_null(eg.material_manager);
        delete_and_null(eg.mesh_manager);
        delete_and_null(eg.shader_manager);
        delete_and_null(eg.texture_manager);
        delete_and_null(eg.fs);
        delete_and_null(eg.time);
        eg.engine_ = nullptr;

        glfwTerminate();
    }

    void process_input()
    {
        update_mouse();

        if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            exit_ = true;
        }

        if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
        {
            glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        float speed = 2.0f;
        if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            speed *= 2;
        }

        const float mouse_speed = 0.0015f;

        const float dt = eg.time->getDelta();

        glm::vec4 delta_pos{0.0f};
        if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS)
        {
            delta_pos.z -= speed * dt;
        }
        if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS)
        {
            delta_pos.z += speed * dt;
        }
        if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS)
        {
            delta_pos.x -= speed * dt;
        }
        if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS)
        {
            delta_pos.x += speed * dt;
        }
        if (glfwGetKey(window_, GLFW_KEY_E) == GLFW_PRESS)
        {
            delta_pos.y += speed * dt;
        }
        if (glfwGetKey(window_, GLFW_KEY_Q) == GLFW_PRESS)
        {
            delta_pos.y -= speed * dt;
        }

        if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            pitch_ -= mouse_delta_y_ * mouse_speed;
            yaw_ -= mouse_delta_x_ * mouse_speed;
        }

        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), yaw_, glm::vec3(0.0f, 1.0f, 0.0f))
            * glm::rotate(glm::mat4(1.0f), pitch_, glm::vec3(1.0f, 0.0f, 0.0f));
        camera_pos_ += glm::vec3(rot * delta_pos);
        camera_.setTransform(glm::translate(glm::mat4{1.0f}, camera_pos_) * rot);
    }

    void update_mouse()
    {
        double x, y;
        glfwGetCursorPos(window_, &x, &y);
        mouse_delta_x_ = x - mouse_pos_x_;
        mouse_delta_y_ = y - mouse_pos_y_;
        mouse_pos_x_ = x;
        mouse_pos_y_ = y;
    }

    void framebuffer_size_callback(GLFWwindow *window, int width, int height)
    {
        glViewport(0, 0, width, height);
        update_proj(window);
    }

    void update_proj(GLFWwindow *window)
    {
        int width = 0;
        int height = 0;
        glfwGetWindowSize(window, &width, &height);
        width = std::max(1, width);
        height = std::max(1, height);
        camera_.setPerspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);
    }

private:
    glm::vec3 camera_pos_{0.0f, 0.0f, 3.0f};
    float pitch_{0.0f};
    float yaw_{0.0f};

    Camera camera_;

    bool exit_{false};
    GLFWwindow *window_{};

    double mouse_pos_x_{0};
    double mouse_pos_y_{0};
    double mouse_delta_x_{0};
    double mouse_delta_y_{0};

    float last_update_fps_time_{0.0f};
};

int main()
{
    Engine game;
    game.exec();
    return 0;
}
