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
#include "Intersection.h"
#include "Light.h"
#include "Material.h"
#include "MaterialManager.h"
#include "Mesh.h"
#include "MeshLoader.h"
#include "MeshManager.h"
#include "Random.h"
#include "Ray.h"
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
#include "profiler/ScopedProfiler.h"
#include "threads/JobQueue.h"
#include "threads/Thread.h"
#include "threads/Threads.h"
#include "time/Time.h"
#include "voxels/BasicBlocks.h"
#include "voxels/Chunk.h"
#include "voxels/VoxelEngine.h"

#include <NodeMesh.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/mat4x4.hpp"
#include <iostream>
#include <vector>


const unsigned int DEFAULT_WIDTH = 1600;
const unsigned int DEFAULT_HEIGHT = 900;

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
        Texture::LoadParams params;
        params.target_format = Texture::Format::RGB;
        params.wrap = Texture::Wrap::ClampToEdge;
        params.min_filter = Texture::Filter::Linear;
        params.mag_filter = Texture::Filter::Linear;
        cubemap_texture->loadCubemap(filenames, Texture::FlipMode::DontFlip, params);

        eng.renderer->setSkyboxTexture(cubemap_texture);

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
        Texture *cat_texture = eng.texture_manager->create("cat");
        cat_texture->load("cat.png");

        Mesh *deformed_cube_mesh = eng.mesh_manager->create("deformed_cube");
        MeshLoader::loadToMesh("deformed_cube.obj", deformed_cube_mesh);

        Material *basic_material = eng.material_manager->create("basic");
        basic_material->setShaderSource(basic_shader_src);
        basic_material->addTexture("uMaterial.diffuseMap", eng.renderer->getWhiteTexture());
        basic_material->addTexture("uMaterial.specularMap", eng.renderer->getWhiteTexture());
        basic_material->addTexture("uMaterial.emissionMap", eng.renderer->getBlackTexture());
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

        const glm::vec4 init_light_pos = glm::vec4(2.0f, 8.0f, 3.0f, 1.0f);
        float angle = -0.6f;

        eng.gui->getSignalOnRender().connect(ctx, [&] {
            ImGui::Begin("Parameters");
            float angle_deg = glm::degrees(angle);
            if (ImGui::SliderFloat("Time", &angle_deg, 0.0f, 360.0f))
            {
                angle = glm::radians(angle_deg);
            }
            bool use_ao = eng.vox->isAmbientOcclusionEnabled();
            if (ImGui::Checkbox("Voxel Engine Ambient Occlusion", &use_ao))
            {
                eng.vox->setAmbientOcclusionEnabled(use_ao);
            }
            ImGui::End();
            ImGui::ShowDemoWindow(nullptr);
        });

        eng.vox->setSeed(123132);

        while (!exit_)
        {
            Profiler::beginFrame();

            eng.time->update();

            glfwPollEvents();
            eng.input->update();

            process_input();

            if (eng.input->isKeyDown(Key::KEY_F5))
            {
                eng.shader_manager->refreshAll();
            }

            angle += eng.time->getDelta() * 0.01;
            if (angle > glm::two_pi<float>())
            {
                angle -= glm::two_pi<float>();
            }

            const glm::vec3 light_pos = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 0, 1))
                * init_light_pos;
            GlobalLight global_light;
            global_light.dir = glm::normalize(-light_pos);
            eng.renderer->setGlobalLight(global_light);

            math::BoundBox bb;
            bb.setCenterAndSize(camera_pos_ + light_pos, {0.1f, 0.1f, 0.1f});
            eng.visualizer->addBoundBox(bb, glm::vec4(1, 1, 1, 1), false);

            const auto add_axis = [](const glm::vec3 &axis) {
                eng.visualizer->addLine(glm::vec3{0, 0, 0}, axis * 15.0f, glm::vec4{axis, 1.0f});
            };
            add_axis(glm::vec3{1, 0, 0});
            add_axis(glm::vec3{0, 1, 0});
            add_axis(glm::vec3{0, 0, 1});

            anim_time += eng.time->getDelta() * anim_time_multiplier;
            light.pos.x = sin(6 + anim_time * 1.0351) * 1.5f + 0.5f;
            light.pos.y = cos(1 + anim_time * 1.2561) * 1.5f + 1.3f;
            light.pos.z = sin(7 + anim_time * 1.125) * 1.5f + 0.5f;

            // node_deformed_cube->setTransform(
            //     glm::rotate(glm::mat4{1.0f}, float(0.25 * eng.time->getTime()),
            //         glm::vec3(1.0f, 0.0f, 0.0f))
            //     * glm::scale(glm::mat4{1.0f}, glm::vec3{1.0f}));

            node_crate->setTransform(glm::translate(glm::mat4{1.0f}, {2, 0, 2})
                * glm::rotate(glm::mat4{1.0f}, float(0.25 * eng.time->getTime()),
                    glm::vec3(1.0f, 1.0f, 1.0f)));

            node_stickman->setTransform(glm::translate(glm::mat4{1.0f}, glm::vec3{1, 1, 0})
                * glm::scale(glm::mat4{1.0f}, glm::vec3{1.6}));

            node_floor->setTransform(glm::translate(glm::mat4{1.0f}, glm::vec3{0, -1, 0}));

            node_light->setTransform(glm::translate(glm::mat4{1.0f}, light.pos)
                * glm::scale(glm::mat4{1.0f}, glm::vec3{0.08f}));
            light_cube_material->setParameterVec3("uColor", light.color);

            const bool on_window = eng.gui->isWantCaptureMouse();
            const bool left_button_pressed = eng.input->isButtonPressed(Button::BUTTON_LEFT);
            const bool g_down = eng.input->isKeyDown(Key::KEY_F);
            if (!on_window && (left_button_pressed || g_down))
            {
                Ray ray = camera_.getNearFarRay(eng.window->getNormalizedCursorPos());
                glm::vec3 dir_n = glm::normalize(ray.end - ray.begin);

                const float distance = 100.0f;

                eng.visualizer->addLine(camera_.getPosition(), ray.begin + dir_n * distance,
                    glm::vec4{1, 1, 1, 1}, true, 4.0f);

                const VoxelEngine::IntersectionResult result
                    = eng.vox->getIntersection(camera_.getPosition(), dir_n, distance);
                if (result.isValid())
                {
                    eng.vox->setBlockAtPosition(result.glob_pos, BlockInfo(0));
                }
            }

            if (eng.input->isKeyDown(Key::KEY_G))
            {
                ScopedProfiler p("Explosion");

                const glm::ivec3 center_pos = VoxelEngine::toBlockPosition(camera_.getPosition());

                constexpr int EXPLOSION_SIZE = 30;
                for (int off_y = -EXPLOSION_SIZE; off_y <= EXPLOSION_SIZE; ++off_y)
                {
                    for (int off_z = -EXPLOSION_SIZE; off_z <= EXPLOSION_SIZE; ++off_z)
                    {
                        for (int off_x = -EXPLOSION_SIZE; off_x <= EXPLOSION_SIZE; ++off_x)
                        {
                            if (math::isOutsideRadius(off_x, off_y, off_z, EXPLOSION_SIZE))
                            {
                                continue;
                            }
                            const glm::ivec3 pos = center_pos + glm::ivec3{off_x, off_y, off_z};
                            eng.vox->setBlockAtPosition(pos, BlockInfo{0});
                        }
                    }
                }
            }

            ///////////////////////////////////////////////////////////

            eng.queue->finishJobsMainThread();

            edg.editor_->setPlayerPositionInfo(camera_.getPosition());
            eng.vox->update(camera_.getPosition());

            ///////////////////////////////////////////////////////////
            eng.queue->finishJobsMainThread();

            eng.renderer->clearBuffers();

            eng.renderer->renderWorld(&camera_, &light);
            eng.visualizer->update(eng.time->getDelta());
            eng.visualizer->render(camera_.getViewProj());
            eng.gui->render();
            eng.window->swap();
            eng.stat.finishFrame();

            Profiler::endFrame();
        }

        shutdown();
    }

private:
    void init()
    {
        Threads::init();

        Profiler::setMaxRecordedFrames(100);
        Profiler::init();

        Random::init();
        eng.engine_ = this;
        eng.queue = new tbb::JobQueue();
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
        eng.vox = new VoxelEngine();

        eng.window = eng.proxy->createWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "RealEngine");

        eng.gui = new Gui();

        eng.visualizer = new Visualizer();

        // Post initialization
        eng.renderer->init();
        eng.vox->init();

        // Editor
        edg.editor_ = new Editor();

        // Post initialization
        eng.queue->runWorkers();
    }

    void shutdown()
    {
        const auto delete_and_null = [](auto &ptr) {
            assert(ptr);
            delete ptr;
            ptr = nullptr;
        };

        // Pre shutdown
        eng.queue->stopWorkers();

        // Editor
        delete_and_null(edg.editor_);

        // Engine
        delete_and_null(eng.visualizer);
        delete_and_null(eng.vox);
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
        delete_and_null(eng.queue);
        Threads::shutdown();
        eng.engine_ = nullptr;
    }

    void process_input()
    {
        if (eng.input->isKeyDown(Key::KEY_ESCAPE))
        {
            exit_ = true;
        }

        if (eng.input->isKeyPressed(Key::KEY_F11))
        {
            const char *path = "profiler.html";
            Profiler::dumpHTML(path);
            std::string message = "Profiler saved to ";
            message += path;
            std::cout << message << std::endl;
            edg.editor_->addPopup(message.c_str());
        }

        if (eng.input->isKeyPressed(Key::KEY_F3))
        {
            eng.world->disableAll();
        }

        if (eng.input->isButtonDown(Button::BUTTON_RIGHT))
        {
            const int wheel = eng.input->getWheel();
            if (wheel > 0)
            {
                camera_speed_ *= 1.1;
            }
            else if (wheel < 0)
            {
                camera_speed_ /= 1.1;
            }
        }

        const bool right_btn = eng.input->isButtonDown(Button::BUTTON_RIGHT);
        eng.input->setMouseGrabbed(right_btn);

        float speed = camera_speed_;
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
        camera_pos_.y = glm::clamp(camera_pos_.y, -100.0f, 1000.0f);
        camera_.setTransform(glm::translate(glm::mat4{1.0f}, camera_pos_) * rot);
    }

    void update_proj(Window *window)
    {
        int width = window->getWidth();
        int height = window->getHeight();
        width = std::max(1, width);
        height = std::max(1, height);
        camera_.setPerspective(75.0f, (float)width / (float)height, 0.1f, 100'000.0f);
    }

private:
    glm::vec3 camera_pos_{20.0f, 130.0f, 20.0f};
    float pitch_{0.0f};
    float yaw_{0.0f};

    Camera camera_;
    float camera_speed_{10.f};

    bool exit_{false};
};

int main()
{
    Engine game;
    game.exec();
    return 0;
}
