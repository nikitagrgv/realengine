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
#include "Texture.h"
#include "TextureManager.h"
#include "VertexArrayObject.h"
#include "VertexBufferObject.h"
#include "Visualizer.h"
#include "World.h"
#include "fs/FileSystem.h"
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
        init();
        glfwMaximizeWindow(window_);


        ///////////////////////////////////////////////////////////////////////////////
        Shader *light_cube_shader = eng.shader_manager->create("light_cube");
        light_cube_shader->loadFile("light_cube.shader");

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
        Shader *shader = eng.shader_manager->create("basic");
        shader->loadFile("shader.shader");

        ///////////////////////////////////////////////////////////////////////////////
        Texture *cat_texture = eng.texture_manager->create();
        cat_texture->load("image.png");
        Mesh *cat_mesh = eng.mesh_manager->create("cat");
        MeshLoader::loadToMesh("object.obj", cat_mesh);

        Material *cat_material = eng.material_manager->create("cat");
        cat_material->setShader(shader);
        cat_material->addTexture("uTexture", cat_texture);
        cat_material->addParameterFloat("uMaterial.shininess", 32.0f);
        cat_material->addDefine("USE_AMBIENT", true);
        cat_material->addDefine("USE_DIFFUSE", true);
        cat_material->addDefine("USE_SPECULAR", true);

        Material *light_cube_material = eng.material_manager->create("light cube");
        light_cube_material->setShader(light_cube_shader);
        light_cube_material->addParameterVec3("uColor");
        light_cube_material->setTwoSided(true);

        ////////////////////////////////////////////////
        Texture *stickman_texture = eng.texture_manager->create();
        stickman_texture->load("image2.png");

        Mesh *stickman_mesh = eng.mesh_manager->create("stickman");
        MeshLoader::loadToMesh("stickman.obj", stickman_mesh, true);

        ////////////////////////////////////////////////
        Texture *floor_texture = eng.texture_manager->create();
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
        update_proj(window_);

        auto node_cat = eng.world->createNode<NodeMesh>();
        node_cat->setName("cat");
        node_cat->setMaterial(cat_material);
        node_cat->setMesh(cat_mesh);

        auto node_stickman = eng.world->createNode<NodeMesh>();
        node_stickman->setName("stickman");
        node_stickman->setMaterial(cat_material);
        node_stickman->setMesh(stickman_mesh);

        auto node_floor = eng.world->createNode<NodeMesh>();
        node_floor->setName("floor");
        node_floor->setMaterial(cat_material);
        node_floor->setMesh(floor_mesh);

        auto node_light = eng.world->createNode<NodeMesh>();
        node_light->setName("light");
        node_light->setMaterial(light_cube_material);
        node_light->setMesh(light_cube_mesh);

        float anim_time = 0.0f;

        float anim_time_multiplier = 1.0f;

        Light light;
        light.pos = glm::vec3{1, 1, 1};
        light.color = glm::vec3{0.2, 0.65, 0.65};
        light.ambient_power = 0.1f;
        light.diffuse_power = 1.0f;
        light.specular_power = 1.0f;

        while (!exit_)
        {
            eng.time->update();


            glfwPollEvents();

            //////////////////////////////////////////////// IMGUI
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            {
                ImGui::Begin("Parameters");
                ImGui::SliderFloat("Time multiplier", &anim_time_multiplier, 0.0f, 10.0f);
                ImGui::ColorEdit3("Light color", glm::value_ptr(light.color),
                    ImGuiColorEditFlags_Float);
                ImGui::SliderFloat("Ambient power", &light.ambient_power, 0.0f, 1.0f);
                ImGui::SliderFloat("Diffuse power", &light.diffuse_power, 0.0f, 1.0f);
                ImGui::SliderFloat("Specular power", &light.specular_power, 0.0f, 1.0f);
                ImGui::End();
            }

            ImGui::ShowDemoWindow(nullptr);

            edg.editor_->render();

            ImGui::Render();
            //////////////////////////////////////////////// IMGUI

            process_input();

            if (glfwGetKey(window_, GLFW_KEY_F5) == GLFW_PRESS)
            {
                eng.shader_manager->recompileAll();
            }

            const auto add_axis = [](const glm::vec3 &axis) {
                eng.visualizer->addLine(glm::vec3{0, 0, 0}, axis, glm::vec4{axis, 1.0f});
            };
            add_axis(glm::vec3{1, 0, 0});
            add_axis(glm::vec3{0, 1, 0});
            add_axis(glm::vec3{0, 0, 1});

            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            anim_time += eng.time->getDelta() * anim_time_multiplier;
            light.pos.x = sin(6 + anim_time * 1.0351) * 1.5f + 0.5f;
            light.pos.y = cos(1 + anim_time * 1.2561) * 1.5f + 1.3f;
            light.pos.z = sin(7 + anim_time * 1.125) * 1.5f + 0.5f;

            node_cat->setTransform(glm::rotate(glm::mat4{1.0f}, float(0.25 * eng.time->getTime()),
                                       glm::vec3(1.0f, 0.0f, 0.0f))
                * glm::scale(glm::mat4{1.0f}, glm::vec3{0.5f}));

            node_stickman->setTransform(glm::translate(glm::mat4{1.0f}, glm::vec3{1, 1, 0})
                * glm::scale(glm::mat4{1.0f}, glm::vec3{0.016f}));

            node_floor->setTransform(glm::translate(glm::mat4{1.0f}, glm::vec3{0, -1, 0}));

            node_light->setTransform(glm::translate(glm::mat4{1.0f}, light.pos)
                * glm::scale(glm::mat4{1.0f}, glm::vec3{0.08f}));
            light_cube_material->setParameterVec3("uColor", light.color);

            eng.renderer->renderWorld(&camera_, &light);



            ////////////////////////////////////////////////
            eng.visualizer->render(camera_.getViewProj());


            ////////////////// IMGUI
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            ////////////////// IMGUI

            glfwSwapBuffers(window_);

            if (glfwWindowShouldClose(window_))
            {
                exit_ = true;
            }
        }
    }

private:
    void init()
    {
        Random::init();
        eng.engine_ = this;
        eng.time = new Time();
        eng.fs = new FileSystem();
        eng.texture_manager = new TextureManager();
        eng.shader_manager = new ShaderManager();
        eng.mesh_manager = new MeshManager();
        eng.material_manager = new MaterialManager();
        eng.renderer = new Renderer();
        eng.world = new World();

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
            eng.engine_->framebuffer_size_callback(window, width, height);
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
        eng.engine_ = nullptr;

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

        const float dt = eng.time->getDelta();

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
};

int main()
{
    Engine game;
    game.exec();
    return 0;
}
