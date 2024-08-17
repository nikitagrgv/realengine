// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "EngineGlobals.h"
#include "fs/FileSystem.h"

#include "glm/mat4x4.hpp"
#include <iostream>

const unsigned int DEFAULT_WIDTH = 800;
const unsigned int DEFAULT_HEIGHT = 600;

class Game
{
public:
    void exec()
    {
        init();

        float vertices[] = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.0f,  0.5f, 0.0f
        };

        unsigned int VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        std::string vertex;
        std::string fragment;
        read_shader("shader.shader", vertex, fragment);


        while (!exit_)
        {
            process_input();

            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glfwSwapBuffers(window_);
            glfwPollEvents();

            if (glfwWindowShouldClose(window_))
            {
                exit_ = true;
            }
        }
    }

private:
    void read_shader(const char *path, std::string &vertex, std::string &fragment)
    {
        vertex.clear();
        fragment.clear();

        std::string shaders = engine_globals.fs->readFile(path);
        const int vertex_idx = shaders.find("#vertex");
        const int fragment_idx = shaders.find("#fragment");
        const int vertex_idx_end = vertex_idx + strlen("#vertex");
        const int fragment_idx_end = fragment_idx + strlen("#fragment");

        assert(vertex_idx < fragment_idx);

        std::string common = shaders.substr(0, vertex_idx);
        vertex = common + shaders.substr(vertex_idx_end, fragment_idx - vertex_idx_end);
        fragment = common + shaders.substr(fragment_idx_end);
    }

    void init()
    {
        engine_globals.fs = new FileSystem();

        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window_ = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "LearnOpenGL", NULL, NULL);
        if (window_ == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
        }
        glfwMakeContextCurrent(window_);
        glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
        }
    }

    void shutdown()
    {
        const auto delete_and_null = [](auto &ptr) {
            assert(ptr);
            delete ptr;
            ptr = nullptr;
        };
        delete_and_null(engine_globals.fs);

        glfwTerminate();
    }

    void process_input()
    {
        if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            exit_ = true;
        }
    }

    static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
    {
        glViewport(0, 0, width, height);
    }

private:
    bool exit_{false};
    GLFWwindow *window_{};
};

int main()
{
    Game game;
    game.exec();
    return 0;
}
