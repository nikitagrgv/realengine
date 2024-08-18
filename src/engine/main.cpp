// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "EngineGlobals.h"
#include "Image.h"
#include "Shader.h"
#include "fs/FileSystem.h"
#include "time/Time.h"

#include "glm/mat4x4.hpp"
#include <iostream>
#include <vector>

const unsigned int DEFAULT_WIDTH = 800;
const unsigned int DEFAULT_HEIGHT = 600;

template<typename V>
class TemplateMesh
{
public:
    TemplateMesh()
    {
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glGenBuffers(1, &ebo_);
    }

    ~TemplateMesh()
    {
        glDeleteVertexArrays(1, &vao_);
        glDeleteBuffers(1, &vbo_);
        glDeleteBuffers(1, &ebo_);
    }

    int addVertex(const V &v)
    {
        vertices_.push_back(v);
        return vertices_.size() - 1;
    }

    int addIndex(unsigned int i)
    {
        indices_.push_back(i);
        return indices_.size() - 1;
    }

    const V &getVertex(int index) const { return vertices_[index]; }
    void setVertex(const V &v, int index) { vertices_[index] = v; }

    int getNumVertices() const { return vertices_.size(); }
    int getNumIndices() const { return indices_.size(); }

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

    void addAttributeFloat(int count)
    {
        Attribute attr;
        attr.count = count;
        attr.size_of_type = sizeof(float);
        attr.type = GL_FLOAT;
        attributes_.push_back(attr);
    }

    void flush()
    {
        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, vertices_.size() * VERTEX_SIZE, vertices_.data(),
            GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * INDEX_SIZE, indices_.data(),
            GL_STATIC_DRAW);

        flush_attributes();

        glBindBuffer(GL_ARRAY_BUFFER, 0); // TODO# remove?
        glBindVertexArray(0);
    }

    void bind() { glBindVertexArray(vao_); }

private:
    void flush_attributes()
    {
        int stride = 0;
        for (const Attribute &attribute : attributes_)
        {
            stride += attribute.count * attribute.size_of_type;
        }

        size_t offset = 0;
        for (int i = 0; i < attributes_.size(); ++i)
        {
            const Attribute &attribute = attributes_[i];
            glVertexAttribPointer(i, attribute.count, attribute.type, GL_FALSE, stride,
                reinterpret_cast<void *>(offset));
            glEnableVertexAttribArray(i);
            offset += attribute.count * attribute.size_of_type;
        }
    }

protected:
    static constexpr int VERTEX_SIZE = sizeof(V);
    static constexpr int INDEX_SIZE = sizeof(unsigned int);

    struct Attribute
    {
        int type{-1};
        int size_of_type{-1};
        int count{-1};
    };
    std::vector<Attribute> attributes_;
    std::vector<V> vertices_;
    std::vector<unsigned int> indices_;

    unsigned int vbo_{0};
    unsigned int vao_{0};
    unsigned int ebo_{0};
};

class Engine
{
public:
    void exec()
    {
        init();

        Shader shader("shader.shader");

        struct Vertex
        {
            float x, y, z;
            float r, g, b;
            float u, v;
        };

        TemplateMesh<Vertex> mesh;
        mesh.addAttributeFloat(3);
        mesh.addAttributeFloat(3);
        mesh.addAttributeFloat(2);

        mesh.addVertex({0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f});
        mesh.addVertex({0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f});
        mesh.addVertex({-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f});
        mesh.addVertex({-0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f});

        mesh.addIndices({0, 1, 3, 1, 2, 3});
        mesh.flush();

        ///////////////////////////////////////////////////////////////////////////////
        TemplateMesh<Vertex> mesh2;
        mesh2.addAttributeFloat(3);
        mesh2.addAttributeFloat(3);

        mesh2.addVertex({0.75f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f});
        mesh2.addVertex({0.75f, 0.25f, 0.0f, 0.0f, 1.0f, 0.0f});
        mesh2.addVertex({0.8f, 00.0f, 0.0f, 0.0f, 0.0f, 1.0f});

        mesh2.addIndices({0, 1, 2});

        mesh2.flush();


        Image image1("image.png");
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image1.getWidth(), image1.getHeight(), 0, GL_RGB,
            GL_UNSIGNED_BYTE, image1.getData());
        glGenerateMipmap(GL_TEXTURE_2D);

        while (!exit_)
        {
            engine_globals.time->update();

            process_input();

            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            shader.bind();
            shader.setUniformFloat("uTime", engine_globals.time->getTime());

            glBindTexture(GL_TEXTURE_2D, texture);

            mesh.bind();
            glDrawElements(GL_TRIANGLES, mesh.getNumIndices(), GL_UNSIGNED_INT, 0);

            mesh2.bind();
            glDrawElements(GL_TRIANGLES, mesh2.getNumIndices(), GL_UNSIGNED_INT, 0);

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
        engine_globals.engine_ = this;
        engine_globals.time = new Time();
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
        glfwSetFramebufferSizeCallback(window_, [](GLFWwindow *window, int width, int height) {
            engine_globals.engine_->framebuffer_size_callback(window, width, height);
        });

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
        delete_and_null(engine_globals.time);
        engine_globals.engine_ = nullptr;

        glfwTerminate();
    }

    void process_input()
    {
        if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            exit_ = true;
        }
    }

    void framebuffer_size_callback(GLFWwindow *window, int width, int height)
    {
        glViewport(0, 0, width, height);
    }

private:
    float pitch_{0.0f};
    float yaw_{0.0f};

    bool exit_{false};
    GLFWwindow *window_{};
};

int main()
{
    Engine game;
    game.exec();
    return 0;
}
