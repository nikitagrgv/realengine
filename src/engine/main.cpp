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

const unsigned int DEFAULT_WIDTH = 1600;
const unsigned int DEFAULT_HEIGHT = 900;

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

    void clearMesh()
    {
        vertices_.clear();
        indices_.clear();
    }

    void clearAttributes() { attributes_.clear(); }

    void clearAll()
    {
        clearMesh();
        clearAttributes();
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
    Visualizer()
    {
        lines_.addAttributeFloat(3); // pos
        lines_.addAttributeFloat(4); // color

        const char *vertex_shader = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec4 aColor;
            out vec4 ioColor;
            uniform mat4 uViewProj;
            void main()
            {
                gl_Position = uViewProj * vec4(aPos, 1.0f);
                ioColor = aColor;
            })";
        const char *fragment_shader = R"(
            #version 330 core
            out vec4 FragColor;
            in vec4 ioColor;
            void main()
            {
                FragColor = ioColor;
            })";
        shader.loadSources(vertex_shader, fragment_shader);
    }

    void addLine(const glm::vec3 &s0, const glm::vec3 &s1, const glm::vec4 &color)
    {
        LinePoint p;
        p.color = color;
        p.pos = s0;
        int v = lines_.addVertex(p);
        lines_.addIndex(v);
        p.pos = s1;
        v = lines_.addVertex(p);
        lines_.addIndex(v);
    }

    void addLine(const glm::vec3 &s0, const glm::vec3 &s1, const glm::vec4 &color0,
        const glm::vec4 &color1)
    {
        LinePoint p;
        p.color = color0;
        p.pos = s0;
        int v = lines_.addVertex(p);
        lines_.addIndex(v);
        p.color = color1;
        p.pos = s1;
        v = lines_.addVertex(p);
        lines_.addIndex(v);
    }

    void render(const glm::mat4 &viewproj)
    {
        lines_.flush(true);
        shader.bind();
        shader.setUniformMat4("uViewProj", viewproj);
        lines_.bind();
        glDrawElements(GL_LINES, lines_.getNumIndices(), GL_UNSIGNED_INT, 0);
        lines_.clearMesh();
    }

private:
    struct LinePoint
    {
        glm::vec3 pos;
        glm::vec4 color;
    };

    Shader shader;
    TemplateMesh<LinePoint> lines_;
};

class Camera
{
public:
    Camera()
        : proj_(glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f))
    {}

    Camera(const glm::mat4 &view, const glm::mat4 &proj)
        : view_(view)
        , proj_(proj)
    {
        update_viewproj();
    }

    explicit Camera(const glm::mat4 &proj)
        : proj_(proj)
    {
        update_viewproj();
    }

    glm::mat4 getMVP(const glm::mat4 &model) const { return viewproj_ * model; }

    void setPerspective(float fov_deg, float aspect, float z_near, float z_far)
    {
        setProj(glm::perspective(glm::radians(fov_deg), aspect, z_near, z_far));
    }

    const glm::mat4 &getProj() const { return proj_; }
    void setProj(const glm::mat4 &proj)
    {
        proj_ = proj;
        update_viewproj();
    }

    const glm::mat4 &getView() const { return view_; }
    void setView(const glm::mat4 &view)
    {
        view_ = view;
        transform_ = glm::inverse(view_);
        update_viewproj();
    }

    const glm::mat4 &getTransform() const { return transform_; }
    void setTransform(const glm::mat4 &transform)
    {
        transform_ = transform;
        view_ = glm::inverse(transform_);
        update_viewproj();
    }

    glm::vec3 getPosition() const { return transform_[3]; }

    const glm::mat4 &getViewProj() const { return viewproj_; }

private:
    void update_viewproj() { viewproj_ = proj_ * view_; }

private:
    glm::mat4 transform_{1.0f};
    glm::mat4 view_{1.0f};
    glm::mat4 proj_{1.0f};
    glm::mat4 viewproj_{1.0f};
};

class Engine
{
public:
    void exec()
    {
        init();

        struct Vertex
        {
            glm::vec3 pos{0.0f};
            glm::vec3 norm{0.0f};
            glm::vec2 uv{0.0f};
        };

        ///////////////////////////////////////////////////////////////////////////////
        struct LightCubeVertex
        {
            glm::vec3 pos{0.0f};
        };
        Shader light_cube_shader("light_cube.shader");

        TemplateMesh<LightCubeVertex> light_cube_mesh;
        light_cube_mesh.addAttributeFloat(3); // pos
        // make a thehaeder
        light_cube_mesh.addVertex({
            {0.0f, 1.0f, 0.0f}
        });
        light_cube_mesh.addVertex({
            {1.0f, 0.0f, 0.0f}
        });
        light_cube_mesh.addVertex({
            {-1.0f, 0.0f, 0.0f}
        });
        light_cube_mesh.addVertex({
            {0.0f, 0.0f, 1.0f}
        });
        light_cube_mesh.addIndices({0, 1, 2, 0, 1, 3, 0, 2, 3, 1, 3, 2});
        light_cube_mesh.flush();

        ///////////////////////////////////////////////////////////////////////////////
        Shader shader("shader.shader");

        ///////////////////////////////////////////////////////////////////////////////
        Image cat_image("image.png");
        Texture cat_texture(cat_image);

        glm::mat4 cat_transform = glm::mat4{1.0f};
        TemplateMesh<Vertex> cat_mesh;
        cat_mesh.addAttributeFloat(3);
        cat_mesh.addAttributeFloat(3);
        cat_mesh.addAttributeFloat(2);
        {
            MeshLoader loader("object.obj");
            for (int i = 0; i < loader.getNumVertices(); i++)
            {
                Vertex v;
                v.pos = loader.getVertexPosition(i) * 10.0f;
                v.norm = loader.getVertexNormal(i);
                v.uv = loader.getVertexTextureCoords(i);
                cat_mesh.addVertex(v);
            }
            for (int i = 0; i < loader.getNumIndices(); i++)
            {
                cat_mesh.addIndex(loader.getIndex(i));
            }
        }
        cat_mesh.flush();

        ////////////////////////////////////////////////
        Image stickman_image("image2.png");
        Texture stickman_texture(stickman_image);

        glm::mat4 stickman_transform = glm::mat4{1.0f};
        TemplateMesh<Vertex> stickman_mesh;
        stickman_mesh.addAttributeFloat(3); // pos
        stickman_mesh.addAttributeFloat(3); // norm
        stickman_mesh.addAttributeFloat(2); // uv
        {
            MeshLoader loader("object.obj");
            for (int i = 0; i < loader.getNumVertices(); i++)
            {
                Vertex v;
                v.pos = loader.getVertexPosition(i)* 10.0f;
                v.norm = loader.getVertexNormal(i);
                v.uv = loader.getVertexTextureCoords(i);
                stickman_mesh.addVertex(v);
            }
            for (int i = 0; i < loader.getNumIndices(); i++)
            {
                stickman_mesh.addIndex(loader.getIndex(i));
            }
        }
        stickman_mesh.flush();
        ////////////////////////////////////////////////
        Image floor_image("floor.png");
        Texture floor_texture(floor_image);

        TemplateMesh<Vertex> floor_mesh;
        floor_mesh.addAttributeFloat(3);
        floor_mesh.addAttributeFloat(3);
        floor_mesh.addAttributeFloat(2);
        const float floor_size = 10.0f;
        const float floor_y = 0.0f;
        const float max_text_coord = 10.0f;
        floor_mesh.addVertex(Vertex{
            {-floor_size, floor_y, -floor_size},
            {0, 1, 0},
            {0.0f, 0.0f}
        });
        floor_mesh.addVertex(Vertex{
            {-floor_size, floor_y, floor_size},
            {0, 1, 0},

            {0.0f, max_text_coord}
        });
        floor_mesh.addVertex(Vertex{
            {floor_size, floor_y, floor_size},
            {0, 1, 0},

            {max_text_coord, max_text_coord}
        });
        floor_mesh.addVertex(Vertex{
            {floor_size, floor_y, -floor_size},
            {0, 1, 0},

            {max_text_coord, 0.0f}
        });
        floor_mesh.addIndices({0, 1, 3, 1, 2, 3});
        floor_mesh.flush();
        ////////////////////////////////////////////////

        camera_.setTransform(glm::translate(glm::mat4{1.0f}, glm::vec3(0.0f, 0.0f, 3.0f)));
        update_proj(window_);

        const auto visualize_normals = [](const TemplateMesh<Vertex> &mesh,
                                           const glm::mat4 &transform) {
            const auto to_local = [&](const glm::vec3 &v) {
                return transform * glm::vec4(v, 1);
            };
            for (int i = 0; i < mesh.getNumVertices(); i++)
            {
                const Vertex &v = mesh.getVertex(i);
                engine_globals.visualizer->addLine(to_local(v.pos), to_local(v.pos + v.norm),
                    {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f});
            }
        };

        glm::vec3 light_color{1.0f};
        glm::vec3 light_pos{1, 1, 1};
        while (!exit_)
        {
            engine_globals.time->update();

            process_input();

            if (glfwGetKey(window_, GLFW_KEY_F5) == GLFW_PRESS)
            {
                shader.recompile();
                light_cube_shader.recompile();
            }

            const auto add_axis = [](const glm::vec3 &axis) {
                engine_globals.visualizer->addLine(glm::vec3{0, 0, 0}, axis, glm::vec4{axis, 1.0f});
            };
            add_axis(glm::vec3{1, 0, 0});
            add_axis(glm::vec3{0, 1, 0});
            add_axis(glm::vec3{0, 0, 1});

            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            const float time = engine_globals.time->getTime();
            const float anim_time = time * 0.1f;
            const float color_anim_time = anim_time * 0.1215f;
            light_color.x = sin(anim_time * 1.1) * 0.4 + 0.6f;
            light_color.y = cos(anim_time * 1.2) * 0.4 + 0.6f;
            light_color.z = sin(anim_time * 1.3) * 0.4 + 0.6f;

            light_pos.x = sin(color_anim_time * 1.1) * 1.5f + 0.5f;
            light_pos.y = cos(color_anim_time * 1.2) * 1.5f + 1.3f;
            light_pos.z = sin(color_anim_time * 1.3) * 1.5f + 0.5f;

            shader.bind();
            shader.setUniformVec3("uLightColor", light_color);
            shader.setUniformVec3("uLightPos", light_pos);
            shader.setUniformVec3("uCameraPos", camera_.getPosition());
            shader.setUniformMat4("uViewProj", camera_.getViewProj());

            glCullFace(GL_BACK);
            ////////////////////////////////////////////////
            cat_transform = glm::rotate(glm::mat4{1.0f},
                                float(0.25 * engine_globals.time->getTime()),
                                glm::vec3(1.0f, 0.0f, 0.0f))
                * glm::scale(glm::mat4{1.0f}, glm::vec3{0.006f});
            shader.setUniformMat4("uModel", cat_transform);
            cat_texture.bind();
            cat_mesh.bind();
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glDrawElements(GL_TRIANGLES, cat_mesh.getNumIndices(), GL_UNSIGNED_INT, 0);

            ////////////////////////////////////////////////
            stickman_texture.bind();
            stickman_mesh.bind();
            stickman_transform = glm::translate(glm::mat4{1.0f}, glm::vec3{1, 1, 0})
                * glm::scale(glm::mat4{1.0f}, glm::vec3{0.016f});
            shader.setUniformMat4("uModel", stickman_transform);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glDrawElements(GL_TRIANGLES, stickman_mesh.getNumIndices(), GL_UNSIGNED_INT, 0);

            ////////////////////////////////////////////////
            floor_texture.bind();
            floor_mesh.bind();
            shader.setUniformMat4("uModel", glm::translate(glm::mat4{1.0f}, glm::vec3{0, -1, 0}));
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glDrawElements(GL_TRIANGLES, floor_mesh.getNumIndices(), GL_UNSIGNED_INT, 0);

            ////////////////////////////////////////////////
            light_cube_shader.bind();
            light_cube_shader.setUniformVec3("uColor", light_color);
            light_cube_shader.setUniformMat4("uMVP",
                camera_.getMVP(glm::translate(glm::mat4{1.0f}, light_pos)
                    * glm::scale(glm::mat4{1.0f}, glm::vec3{0.08f})));
            light_cube_mesh.bind();
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDrawElements(GL_TRIANGLES, light_cube_mesh.getNumIndices(), GL_UNSIGNED_INT, 0);

            ///////////////////////////////////////////////
            visualize_normals(cat_mesh, cat_transform);
            visualize_normals(stickman_mesh, stickman_transform);
            visualize_normals(floor_mesh, glm::translate(glm::mat4{1.0f}, glm::vec3{0, -1, 0}));
            ////////////////////////////////////////////////
            engine_globals.visualizer->render(camera_.getViewProj());

            glfwSwapBuffers(window_);
            glfwPollEvents();

            if (glfwWindowShouldClose(window_))
            {
                exit_ = true;
            }

            if (last_update_fps_time_ < engine_globals.time->getTime() - 1.0f)
            {
                last_update_fps_time_ = engine_globals.time->getTime();
                glfwSetWindowTitle(window_,
                    std::string("FPS: " + std::to_string(engine_globals.time->getFps())).c_str());
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
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // TODO DEBUG

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

        engine_globals.visualizer = new Visualizer();
    }

    void shutdown()
    {
        const auto delete_and_null = [](auto &ptr) {
            assert(ptr);
            delete ptr;
            ptr = nullptr;
        };
        delete_and_null(engine_globals.visualizer);
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

        float speed = 2.0f;
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
