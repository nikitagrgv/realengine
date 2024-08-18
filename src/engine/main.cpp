// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "EngineGlobals.h"
#include "Image.h"
#include "MeshLoader.h"
#include "Shader.h"
#include "Texture.h"
#include "fs/FileSystem.h"
#include "time/Time.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
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

    void flush(bool dynamic = false)
    {
        const int load_flag = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, vertices_.size() * VERTEX_SIZE, vertices_.data(), load_flag);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * INDEX_SIZE, indices_.data(),
            load_flag);

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

class Visualizer
{
public:


private:
    Shader shader;
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
            float u, v;
        };

        TemplateMesh<Vertex> mesh;
        mesh.addAttributeFloat(3);
        mesh.addAttributeFloat(2);
        mesh.addVertex({0.5f, 0.5f, 0.0f, 1.0f, 1.0f});
        mesh.addVertex({0.5f, -0.5f, 0.0f, 1.0f, 0.0f});
        mesh.addVertex({-0.5f, -0.5f, 0.0f, 0.0f, 0.0f});
        mesh.addVertex({-0.5f, 0.5f, 0.0f, 0.0f, 1.0f});
        mesh.addIndices({0, 1, 3, 1, 2, 3});
        mesh.flush();

        ///////////////////////////////////////////////////////////////////////////////
        MeshLoader loader("stickman.obj");

        TemplateMesh<Vertex> mesh2;
        mesh2.addAttributeFloat(3); // pos
        mesh2.addAttributeFloat(2); // uv
        for (int i = 0; i < loader.getNumVertices(); i++)
        {
            Vertex v;
            v.x = loader.getVertexPosition(i).x;
            v.y = loader.getVertexPosition(i).y;
            v.z = loader.getVertexPosition(i).z;
            v.u = loader.getVertexTextureCoords(i).x;
            v.v = loader.getVertexTextureCoords(i).y;
            mesh2.addVertex(v);
        }
        for (int i = 0; i < loader.getNumIndices(); i++)
        {
            mesh2.addIndex(loader.getIndex(i));
        }
        mesh2.flush();

        Image image1("image2.png");
        Texture texture1(image1);

        Image image2("image.png");
        Texture texture2(image2);

        camera_ = glm::translate(camera_, glm::vec3(0.0f, 0.0f, 3.0f));
        update_proj(window_);

        while (!exit_)
        {
            engine_globals.time->update();

            process_input();

            view_ = glm::inverse(camera_);

            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader.bind();

            glm::mat4 matr = glm::rotate(glm::mat4{1.0f}, 0 * float(engine_globals.time->getTime()),
                glm::vec3(0.8f, 0.8f, 1.0f));
            shader.setUniformMat4("uModel", matr);
            shader.setUniformMat4("uView", view_);
            shader.setUniformMat4("uProj", proj_);
            texture1.bind();
            mesh.bind();
            glEnable(GL_DEPTH_TEST);
            glDrawElements(GL_TRIANGLES, mesh.getNumIndices(), GL_UNSIGNED_INT, 0);

            texture2.bind();
            mesh2.bind();
            shader.setUniformMat4("uModel",
                glm::translate(glm::mat4{1.0f}, glm::vec3{2, 2, 0})
                    * glm::scale(glm::mat4{1.0f}, glm::vec3{0.05f}));
            glEnable(GL_DEPTH_TEST);
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

        update_mouse();
        mouse_delta_x_ = 0;
        mouse_delta_y_ = 0;
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

        float speed = 1.0f;
        if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            speed *= 2;
        }

        const float mouse_speed = 0.0015f;

        const float dt = engine_globals.time->getDelta();

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
        camera_ = glm::translate(glm::mat4{1.0f}, camera_pos_) * rot;
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
        proj_ = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    }

private:
    glm::vec3 camera_pos_{0.0f, 0.0f, 3.0f};
    float pitch_{0.0f};
    float yaw_{0.0f};

    glm::mat4 camera_{1.0f};
    glm::mat4 view_{1.0f};
    glm::mat4 proj_{1.0f};

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
