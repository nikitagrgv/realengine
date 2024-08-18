// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "EngineGlobals.h"
#include "fs/FileSystem.h"

#include "glm/mat4x4.hpp"
#include <iostream>
#include <vector>

const unsigned int DEFAULT_WIDTH = 800;
const unsigned int DEFAULT_HEIGHT = 600;

class Shader
{
public:
    REMOVE_COPY_MOVE_CLASS(Shader);

    Shader(const char *path)
    {
        std::string vertex_source;
        std::string fragment_source;
        read_shader("shader.shader", vertex_source, fragment_source);
        const char *vertex_ptr = vertex_source.c_str();
        const char *fragment_ptr = fragment_source.c_str();

        unsigned int vertex_id;
        vertex_id = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_id, 1, &vertex_ptr, NULL);
        glCompileShader(vertex_id);
        valid_ = check_compiler_errors(vertex_id);
        if (!valid_)
        {
            glDeleteShader(vertex_id);
            return;
        }

        unsigned int fragment_id;
        fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_id, 1, &fragment_ptr, NULL);
        glCompileShader(fragment_id);
        valid_ = check_compiler_errors(fragment_id);
        if (!valid_)
        {
            glDeleteShader(vertex_id);
            glDeleteShader(fragment_id);
            return;
        }

        program_id_ = glCreateProgram();
        glAttachShader(program_id_, vertex_id);
        glAttachShader(program_id_, fragment_id);
        glLinkProgram(program_id_);
        valid_ = check_linking_errors(program_id_);
        if (!valid_)
        {
            glDeleteShader(vertex_id);
            glDeleteShader(fragment_id);
            glDeleteProgram(program_id_);
            return;
        }

        glUseProgram(program_id_);
        glDeleteShader(vertex_id);
        glDeleteShader(fragment_id);
    }

    ~Shader() { clear(); }

    void clear()
    {
        if (valid_)
        {
            glDeleteProgram(program_id_);
            program_id_ = 0;
            valid_ = false;
        }
    }

    void bind() { glUseProgram(program_id_); }

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

        const auto replace_with = [](std::string &string, const char* from, const char* to)
        {
            std::string::size_type pos = 0;
            while ((pos = string.find(from, pos)) != std::string::npos)
            {
                string.replace(pos, strlen(from), to);
                pos += strlen(to);
            }
        };

        vertex = common;
        replace_with(vertex, "#inout", "out");
        vertex += shaders.substr(vertex_idx_end, fragment_idx - vertex_idx_end);

        fragment = std::move(common);
        replace_with(fragment, "#inout", "in");
        fragment += shaders.substr(fragment_idx_end);
    }

    bool check_compiler_errors(unsigned int shader)
    {
        int success;
        char infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        return success;
    }

    bool check_linking_errors(unsigned int program)
    {
        int success;
        char infoLog[512];
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::LINKING_FAILED\n" << infoLog << std::endl;
        }
        return success;
    }

private:
    bool valid_{false};
    unsigned int program_id_{0};
};


struct Vertex
{
    float x, y, z;
    float r, g, b;
};

template<typename V>
class Mesh
{
public:
    Mesh()
    {
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glGenBuffers(1, &ebo_);
        glBindVertexArray(vao_);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0); // TODO# remove?
        glBindVertexArray(0);
    }

    ~Mesh()
    {
        glDeleteVertexArrays(1, &vao_);
        glDeleteBuffers(1, &vbo_);
        glDeleteBuffers(1, &ebo_);
    }

    template<typename A>
    void addVertices(const A &a)
    {
        for (const auto &v : a)
        {
            addVertex(v);
        }
    }

    template<typename A>
    void addIndices(const A &a)
    {
        for (const auto &i : a)
        {
            addIndex(i);
        }
    }

    void addIndices(std::initializer_list<unsigned int> list)
    {
        indices_.insert(indices_.end(), list);
    }

    void addVertex(const V &v) { vertices_.push_back(v); }
    void addIndex(unsigned int i) { indices_.push_back(i); }

    void flush()
    {
        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, vertices_.size() * VERTEX_SIZE, vertices_.data(),
            GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * INDEX_SIZE, indices_.data(),
            GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0); // TODO# remove?
        glBindVertexArray(0);
    }

    void bind() { glBindVertexArray(vao_); }

private:
    static constexpr int VERTEX_SIZE = sizeof(V);
    static constexpr int INDEX_SIZE = sizeof(unsigned int);

    std::vector<V> vertices_;
    std::vector<unsigned int> indices_;

    unsigned int vbo_{0};
    unsigned int vao_{0};
    unsigned int ebo_{0};
};


class Game
{
public:
    void exec()
    {
        init();

        Shader shader("shader.shader");

        Mesh<Vertex> mesh;
        mesh.addVertex({0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f});
        mesh.addVertex({0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f});
        mesh.addVertex({-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f});
        mesh.addVertex({-0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f});

        mesh.addIndices({0, 1, 3, 1, 2, 3});

        mesh.addIndex(0);
        mesh.addIndex(1);
        mesh.addIndex(3);
        mesh.addIndex(1);
        mesh.addIndex(2);
        mesh.addIndex(3);
        mesh.flush();

        while (!exit_)
        {
            process_input();

            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            shader.bind();
            mesh.bind();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            glfwSwapBuffers(window_);
            glfwPollEvents();

            if (glfwWindowShouldClose(window_))
            {
                exit_ = true;
            }
        }
    }

private:
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
