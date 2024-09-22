// clang-format off
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "Camera.h"
#include "Editor.h"
#include "EditorGlobals.h"
#include "EngineGlobals.h"
#include "Gui.h"
#include "Image.h"
#include "Light.h"
#include "Material.h"
#include "MaterialManager.h"
#include "Mesh.h"
#include "MeshLoader.h"
#include "MeshManager.h"
#include "Random.h"
#include "Renderer.h"
#include "Shader.h"
#include "ShaderManager.h"
#include "SystemProxy.h"
#include "Texture.h"
#include "TextureManager.h"
#include "VertexArrayObject.h"
#include "VertexBufferObject.h"
#include "Visualizer.h"
#include "Window.h"
#include "World.h"
#include "fs/FileSystem.h"
#include "input/Input.h"
#include "time/Time.h"

#include <NodeMesh.h>

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
        Context ctx;

        init();
        eng.window->maximize();
        eng.window->getSignalCloseRequested().connect(ctx, [this] { exit_ = true; });
        eng.window->getSignalResized().connect(ctx,
            [this](int w, int h) { update_proj(eng.window); });

        ///////////////////////////////////////////////////////////////////////////////
        Texture *cubemap_texture = eng.texture_manager->create("cubemap");
        const char *filenames[6] = {
            "skybox/right.jpg",  //
            "skybox/left.jpg",   //
            "skybox/top.jpg",    //
            "skybox/bottom.jpg", //
            "skybox/front.jpg",  //
            "skybox/back.jpg"    //
        };
        cubemap_texture->loadCubemap(filenames, Texture::Format::RGB, Texture::Wrap::ClampToEdge,
            Texture::Filter::Linear, Texture::Filter::Linear, false);

        // TODO: without mesh
        Mesh *cubemap_mesh = eng.mesh_manager->create("cubemap");
        {
            int v0 = cubemap_mesh->addVertex(glm::vec3{-1, -1, -1});
            int v1 = cubemap_mesh->addVertex(glm::vec3{1, -1, -1});
            int v2 = cubemap_mesh->addVertex(glm::vec3{-1, -1, 1});
            int v3 = cubemap_mesh->addVertex(glm::vec3{1, -1, 1});

            int v4 = cubemap_mesh->addVertex(glm::vec3{-1, 1, -1});
            int v5 = cubemap_mesh->addVertex(glm::vec3{1, 1, -1});
            int v6 = cubemap_mesh->addVertex(glm::vec3{-1, 1, 1});
            int v7 = cubemap_mesh->addVertex(glm::vec3{1, 1, 1});

            cubemap_mesh->addIndices(v1, v2, v3); // -Y
            cubemap_mesh->addIndices(v1, v0, v2);

            cubemap_mesh->addIndices(v5, v6, v7); // +Y
            cubemap_mesh->addIndices(v5, v4, v6);

            cubemap_mesh->addIndices(v4, v0, v2); // -X
            cubemap_mesh->addIndices(v4, v2, v6);

            cubemap_mesh->addIndices(v5, v1, v3); // +X
            cubemap_mesh->addIndices(v5, v3, v7);

            cubemap_mesh->addIndices(v1, v0, v4); // -Z
            cubemap_mesh->addIndices(v1, v5, v4);

            cubemap_mesh->addIndices(v3, v2, v6); // +Z
            cubemap_mesh->addIndices(v3, v7, v6);

            cubemap_mesh->flush();
        }

        ShaderSource *cubemap_shader = eng.shader_manager->create("cubemap");
        cubemap_shader->setFile("base/environment.shader");

        Material *cubemap_material = eng.material_manager->create("cubemap");
        cubemap_material->setShaderSource(cubemap_shader);
        cubemap_material->setTwoSided(true);
        cubemap_material->addTexture("uSkybox");
        cubemap_material->setTexture("uSkybox", cubemap_texture);

        ///////////////////////////////////////////////////////////////////////////////

        ShaderSource *light_cube_shader_src = eng.shader_manager->create("light_cube");
        light_cube_shader_src->setFile("light_cube.shader");

        Mesh *light_cube_mesh = eng.mesh_manager->create();
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
        ShaderSource *basic_shader_src = eng.shader_manager->create("basic");
        basic_shader_src->setFile("base/basic.shader");

        ///////////////////////////////////////////////////////////////////////////////
        Texture *white_texture = eng.texture_manager->create("white");
        white_texture->load("base/white.png");

        Texture *black_texture = eng.texture_manager->create("black");
        black_texture->load("base/black.png");

        Texture *normal_default_texture = eng.texture_manager->create("normal_default");
        normal_default_texture->load("base/normal_default.png");

        ///////////////////////////////////////////////////////////////////////////////
        Texture *cat_texture = eng.texture_manager->create("cat");
        cat_texture->load("cat.png");

        Mesh *deformed_cube_mesh = eng.mesh_manager->create("deformed_cube");
        MeshLoader::loadToMesh("deformed_cube.obj", deformed_cube_mesh);

        Material *basic_material = eng.material_manager->create("basic");
        basic_material->setShaderSource(basic_shader_src);
        basic_material->addTexture("uMaterial.diffuseMap", white_texture);
        basic_material->addTexture("uMaterial.specularMap", white_texture);
        basic_material->addTexture("uMaterial.emissionMap", black_texture);
        basic_material->addParameterVec3("uMaterial.ambient", glm::vec3{1, 1, 1});
        basic_material->addParameterVec3("uMaterial.diffuse", glm::vec3{1, 1, 1});
        basic_material->addParameterVec3("uMaterial.specular", glm::vec3{1, 1, 1});
        // basic_material->addParameterMat4("test");
        basic_material->addParameterFloat("uMaterial.shininess", 32.0f);
        basic_material->addDefine("USE_AMBIENT", true);
        basic_material->addDefine("USE_DIFFUSE", true);
        basic_material->addDefine("USE_SPECULAR", true);

        Material *light_cube_material = eng.material_manager->create("light cube");
        light_cube_material->setShaderSource(light_cube_shader_src);
        light_cube_material->addParameterVec3("uColor");
        light_cube_material->setTwoSided(true);

        ////////////////////////////////////////////////
        Texture *meme_texture = eng.texture_manager->create("meme");
        meme_texture->load("meme.png");

        Mesh *stickman_mesh = eng.mesh_manager->create("stickman");
        MeshLoader::loadToMesh("stickman.obj", stickman_mesh, 0.01, true);

        /////////////////////////////////////////////////
        Mesh *crate_mesh = eng.mesh_manager->create("crate");
        MeshLoader::loadToMesh("crate.obj", crate_mesh, 0.5f);

        Texture *crate_albedo_texture = eng.texture_manager->create("crate_albedo");
        crate_albedo_texture->load("crate_albedo.png");

        Texture *crate_specular_texture = eng.texture_manager->create("crate_specular");
        crate_specular_texture->load("crate_specular.png");

        Texture *crate_emission_texture = eng.texture_manager->create("crate_emission");
        crate_emission_texture->load("crate_emission.png");

        auto crate_mat = eng.material_manager->inherit(basic_material);
        crate_mat->setTexture("uMaterial.diffuseMap", crate_albedo_texture);
        crate_mat->setTexture("uMaterial.specularMap", crate_specular_texture);
        crate_mat->setTexture("uMaterial.emissionMap", crate_emission_texture);

        ////////////////////////////////////////////////
        Texture *floor_texture = eng.texture_manager->create("floor");
        floor_texture->load("floor.png");

        Mesh *floor_mesh = eng.mesh_manager->create();
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
        update_proj(eng.window);

        for (int i = -5; i < 5; ++i)
        {
            std::string name = "cube_" + std::to_string(i);

            auto cube = eng.world->createNode<NodeMesh>();
            cube->setName(name);
            if (i % 2 == 0)
            {
                cube->setMesh(stickman_mesh);
            }
            else
            {
                cube->setMesh(crate_mesh);
            }
            cube->setTransform(glm::translate(
                glm::mat4{
                    1.0f
            },
                {glm::vec3{i * 1.0f, 0.0f, -1.0f}}));

            auto mat = eng.material_manager->inherit(crate_mat, name.c_str());
            cube->setMaterial(mat);
        }


        auto node_deformed_cube = eng.world->createNode<NodeMesh>();
        node_deformed_cube->setName("deformed cube");
        node_deformed_cube->setMaterial(basic_material);
        node_deformed_cube->setMesh(deformed_cube_mesh);

        auto node_crate = eng.world->createNode<NodeMesh>();
        node_crate->setName("crate");
        node_crate->setMaterial(crate_mat);
        node_crate->setMesh(crate_mesh);

        auto node_stickman = eng.world->createNode<NodeMesh>();
        node_stickman->setName("stickman");
        node_stickman->setMaterial(basic_material);
        node_stickman->setMesh(stickman_mesh);

        auto node_floor = eng.world->createNode<NodeMesh>();
        node_floor->setName("floor");
        node_floor->setMaterial(eng.material_manager->inherit(basic_material, "floor"));
        node_floor->setMesh(floor_mesh);

        auto node_light = eng.world->createNode<NodeMesh>();
        node_light->setName("light");
        node_light->setMaterial(light_cube_material);
        node_light->setMesh(light_cube_mesh);

        float anim_time = 0.0f;

        float anim_time_multiplier = 1.0f;

        Light light;
        light.pos = glm::vec3{1, 1, 1};
        light.color = glm::vec3{0.8, 0.88, 0.72};
        light.ambient_power = 0.1f;
        light.diffuse_power = 1.0f;
        light.specular_power = 1.0f;

        eng.gui->getSignalOnUpdate().connect(ctx, [&] {
            ImGui::Begin("Parameters");
            ImGui::SliderFloat("Time multiplier", &anim_time_multiplier, 0.0f, 10.0f);
            ImGui::ColorEdit3("Light color", glm::value_ptr(light.color),
                ImGuiColorEditFlags_Float);
            ImGui::SliderFloat("Ambient power", &light.ambient_power, 0.0f, 1.0f);
            ImGui::SliderFloat("Diffuse power", &light.diffuse_power, 0.0f, 1.0f);
            ImGui::SliderFloat("Specular power", &light.specular_power, 0.0f, 1.0f);
            ImGui::End();

            ImGui::ShowDemoWindow(nullptr);
        });

        while (!exit_)
        {
            eng.time->update();

            glfwPollEvents();
            eng.input->update();

            process_input();

            if (eng.input->isKeyDown(Key::KEY_F5))
            {
                eng.shader_manager->refreshAll();
            }

            const auto add_axis = [](const glm::vec3 &axis) {
                eng.visualizer->addLine(glm::vec3{0, 0, 0}, axis, glm::vec4{axis, 1.0f});
            };
            add_axis(glm::vec3{1, 0, 0});
            add_axis(glm::vec3{0, 1, 0});
            add_axis(glm::vec3{0, 0, 1});

            anim_time += eng.time->getDelta() * anim_time_multiplier;
            light.pos.x = sin(6 + anim_time * 1.0351) * 1.5f + 0.5f;
            light.pos.y = cos(1 + anim_time * 1.2561) * 1.5f + 1.3f;
            light.pos.z = sin(7 + anim_time * 1.125) * 1.5f + 0.5f;

            node_deformed_cube->setTransform(
                glm::rotate(glm::mat4{1.0f}, float(0.25 * eng.time->getTime()),
                    glm::vec3(1.0f, 0.0f, 0.0f))
                * glm::scale(glm::mat4{1.0f}, glm::vec3{0.5f}));

            node_crate->setTransform(glm::translate(glm::mat4{1.0f}, {2, 0, 2})
                * glm::rotate(glm::mat4{1.0f}, float(0.25 * eng.time->getTime()),
                    glm::vec3(1.0f, 1.0f, 1.0f)));

            node_stickman->setTransform(glm::translate(glm::mat4{1.0f}, glm::vec3{1, 1, 0})
                * glm::scale(glm::mat4{1.0f}, glm::vec3{1.6}));

            node_floor->setTransform(glm::translate(glm::mat4{1.0f}, glm::vec3{0, -1, 0}));

            node_light->setTransform(glm::translate(glm::mat4{1.0f}, light.pos)
                * glm::scale(glm::mat4{1.0f}, glm::vec3{0.08f}));
            light_cube_material->setParameterVec3("uColor", light.color);

            ///////////////////////////////////////////////////////////
            eng.renderer->clearBuffers();

            //////////////////
            Shader *s = cubemap_material->getShader();
            if (s->isDirty())
            {
                s->recompile();
            }
            s->bind();
            auto c = camera_.getView();
            c[3] = glm::vec4{0, 0, 0, 1};
            c = camera_.getProj() * c;
            s->setUniformMat4("uViewProj", c);
            cubemap_mesh->bind();
            GL_CHECKED(glDisable(GL_CULL_FACE));
            GL_CHECKED(
                glDrawElements(GL_TRIANGLES, cubemap_mesh->getNumIndices(), GL_UNSIGNED_INT, 0));

            //////////////////

            eng.renderer->renderWorld(&camera_, &light);
            eng.visualizer->render(camera_.getViewProj());
            eng.gui->update();
            eng.gui->swap();
            eng.window->swap();
            eng.stat.finishFrame();
        }
    }

private:
    void init()
    {
        Random::init();
        eng.engine_ = this;
        eng.proxy = new SystemProxy();
        eng.input = new Input();
        eng.time = new Time();
        eng.fs = new FileSystem();
        eng.texture_manager = new TextureManager();
        eng.shader_manager = new ShaderManager();
        eng.mesh_manager = new MeshManager();
        eng.material_manager = new MaterialManager();
        eng.renderer = new Renderer();
        eng.world = new World();

        eng.window = eng.proxy->createWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "RealEngine");

        eng.gui = new Gui();

        eng.visualizer = new Visualizer();

        // Editor
        edg.editor_ = new Editor();
    }

    void shutdown()
    {
        const auto delete_and_null = [](auto &ptr) {
            assert(ptr);
            delete ptr;
            ptr = nullptr;
        };

        // Editor
        delete_and_null(edg.editor_);

        // Engine
        delete_and_null(eng.visualizer);
        delete_and_null(eng.world);
        delete_and_null(eng.renderer);
        delete_and_null(eng.material_manager);
        delete_and_null(eng.mesh_manager);
        delete_and_null(eng.shader_manager);
        delete_and_null(eng.texture_manager);
        delete_and_null(eng.fs);
        delete_and_null(eng.time);
        delete_and_null(eng.input);
        delete_and_null(eng.proxy);
        eng.engine_ = nullptr;
    }

    void process_input()
    {
        if (eng.input->isKeyDown(Key::KEY_ESCAPE))
        {
            exit_ = true;
        }

        const bool right_btn = eng.input->isButtonDown(Button::BUTTON_RIGHT);
        eng.input->setMouseGrabbed(right_btn);

        float speed = 2.0f;
        if (eng.input->isKeyDown(Key::KEY_LEFT_SHIFT))
        {
            speed *= 2;
        }

        const float mouse_speed = 0.0015f;

        const float dt = eng.time->getDelta();

        glm::vec4 delta_pos{0.0f};
        if (eng.input->isKeyDown(Key::KEY_W))
        {
            delta_pos.z -= speed * dt;
        }
        if (eng.input->isKeyDown(Key::KEY_S))
        {
            delta_pos.z += speed * dt;
        }
        if (eng.input->isKeyDown(Key::KEY_A))
        {
            delta_pos.x -= speed * dt;
        }
        if (eng.input->isKeyDown(Key::KEY_D))
        {
            delta_pos.x += speed * dt;
        }
        if (eng.input->isKeyDown(Key::KEY_E))
        {
            delta_pos.y += speed * dt;
        }
        if (eng.input->isKeyDown(Key::KEY_Q))
        {
            delta_pos.y -= speed * dt;
        }

        if (right_btn)
        {
            const glm::vec2 mouse_delta = eng.input->getMouseDelta();
            yaw_ -= mouse_delta.x * mouse_speed;
            pitch_ -= mouse_delta.y * mouse_speed;
        }

        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), yaw_, glm::vec3(0.0f, 1.0f, 0.0f))
            * glm::rotate(glm::mat4(1.0f), pitch_, glm::vec3(1.0f, 0.0f, 0.0f));
        camera_pos_ += glm::vec3(rot * delta_pos);
        camera_.setTransform(glm::translate(glm::mat4{1.0f}, camera_pos_) * rot);
    }

    void update_proj(Window *window)
    {
        int width = window->getWidth();
        int height = window->getHeight();
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
};

int main()
{
    Engine game;
    game.exec();
    return 0;
}
