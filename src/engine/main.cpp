// clang-format off
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
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
class VertexBufferObject
{
public:
    REMOVE_COPY_CLASS(VertexBufferObject);

    static void unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

    VertexBufferObject() { glGenBuffers(1, &vbo_); }

    ~VertexBufferObject()
    {
        if (vbo_ != 0)
        {
            glDeleteBuffers(1, &vbo_);
        }
    }

    VertexBufferObject(VertexBufferObject &&other) noexcept { *this = std::move(other); }

    VertexBufferObject &operator=(VertexBufferObject &&other) noexcept
    {
        if (this != &other)
        {
            if (vbo_ != 0)
            {
                glDeleteBuffers(1, &vbo_);
            }
            vbo_ = other.vbo_;
            other.vbo_ = 0;
            vertices_ = std::move(other.vertices_);
        }
        return *this;
    }

    int addVertex(const V &v)
    {
        const int index = vertices_.size();
        vertices_.push_back(v);
        return index;
    }

    const V &getVertex(int index) const { return vertices_[index]; }
    V &getVertex(int index) { return vertices_[index]; }
    void setVertex(const V &v, int index) { vertices_[index] = v; }

    int getNumVertices() const { return vertices_.size(); }

    void clear() { vertices_.clear(); }

    void bind() const
    {
        if (vbo_ == 0)
        {
            std::cout << "VertexBufferObject::bind() - vbo_ == 0" << std::endl;
            return;
        }
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    }

    void flush(bool dynamic = false)
    {
        if (vbo_ == 0)
        {
            std::cout << "VertexBufferObject::flush() - vbo_ == 0" << std::endl;
            return;
        }
        const int load_flag = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, vertices_.size() * VERTEX_SIZE, vertices_.data(), load_flag);
    }

private:
    static constexpr int VERTEX_SIZE = sizeof(V);

    std::vector<V> vertices_;
    unsigned int vbo_{};
};

class VertexArrayObject
{
public:
    REMOVE_COPY_CLASS(VertexArrayObject);

    static void unbind() { glBindVertexArray(0); }

    VertexArrayObject() { glGenVertexArrays(1, &vao_); }

    ~VertexArrayObject()
    {
        if (vao_ != 0)
        {
            glDeleteVertexArrays(1, &vao_);
        }
    }

    VertexArrayObject(VertexArrayObject &&other) noexcept { *this = std::move(other); }

    VertexArrayObject &operator=(VertexArrayObject &&other) noexcept
    {
        if (this != &other)
        {
            if (vao_ != 0)
            {
                glDeleteVertexArrays(1, &vao_);
            }
            vao_ = other.vao_;
            other.vao_ = 0;
            attributes_ = std::move(other.attributes_);
        }
        return *this;
    }

    void addAttributeFloat(int count)
    {
        Attribute attr;
        attr.count = count;
        attr.size_of_type = sizeof(float);
        attr.type = GL_FLOAT;
        attributes_.push_back(attr);
    }

    void clear() { attributes_.clear(); }

    void bind() const
    {
        if (vao_ == 0)
        {
            std::cout << "VertexArrayObject::bind() - vao_ == 0" << std::endl;
            return;
        }
        glBindVertexArray(vao_);
    }

    void flush() const
    {
        if (vao_ == 0)
        {
            std::cout << "VertexArrayObject::flush() - vao_ == 0" << std::endl;
            return;
        }

        int stride = 0;
        for (const Attribute &attribute : attributes_)
        {
            stride += attribute.count * attribute.size_of_type;
        }

        glBindVertexArray(vao_);
        size_t offset = 0;
        for (int i = 0; i < attributes_.size(); ++i)
        {
            const Attribute &attribute = attributes_[i];
            glVertexAttribPointer(i, attribute.count, attribute.type, GL_FALSE, stride,
                reinterpret_cast<void *>(offset));
            glEnableVertexAttribArray(i);
            offset += attribute.count * attribute.size_of_type;
        }
        unbind();
    }

private:
    struct Attribute
    {
        int type{-1};
        int size_of_type{-1};
        int count{-1};
    };
    std::vector<Attribute> attributes_;
    unsigned int vao_{};
};

class IndexBufferObject
{
public:
    REMOVE_COPY_CLASS(IndexBufferObject);

    static void unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

    IndexBufferObject() { glGenBuffers(1, &ebo_); }

    ~IndexBufferObject() { glDeleteBuffers(1, &ebo_); }

    IndexBufferObject(IndexBufferObject &&other) noexcept { *this = std::move(other); }

    IndexBufferObject &operator=(IndexBufferObject &&other) noexcept
    {
        if (this != &other)
        {
            if (ebo_ != 0)
            {
                glDeleteBuffers(1, &ebo_);
            }
            ebo_ = other.ebo_;
            other.ebo_ = 0;
            indices_ = std::move(other.indices_);
        }
        return *this;
    }

    int addIndex(unsigned int i)
    {
        const int index = indices_.size();
        indices_.push_back(i);
        return index;
    }
    unsigned int getIndex(int i) const { return indices_[i]; }

    int getNumIndices() const { return indices_.size(); }

    void clear() { indices_.clear(); }

    void bind()
    {
        if (ebo_ == 0)
        {
            std::cout << "IndexBufferObject::bind() - ebo_ == 0" << std::endl;
            return;
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    }

    void flush(bool dynamic = false)
    {
        if (ebo_ == 0)
        {
            std::cout << "IndexBufferObject::flush() - ebo_ == 0" << std::endl;
            return;
        }
        const int load_flag = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * INDEX_SIZE, indices_.data(),
            load_flag);
    }

private:
    static constexpr int INDEX_SIZE = sizeof(unsigned int);

    std::vector<unsigned int> indices_;
    unsigned int ebo_{0};
};

class Mesh
{
public:
    Mesh()
    {
        vao_.bind();
        vao_.addAttributeFloat(3); // pos
        vao_.addAttributeFloat(3); // norm
        vao_.addAttributeFloat(2); // uv
        vbo_.bind();
        ebo_.bind();
        vao_.flush();
        VertexArrayObject::unbind();
    }

    // Vertices
    int addVertex() { return vbo_.addVertex(Vertex{}); }

    int addVertex(const glm::vec3 &pos, const glm::vec3 &norm, const glm::vec2 &uv)
    {
        return vbo_.addVertex(Vertex{pos, norm, uv});
    }

    int addVertex(const glm::vec3 &pos, const glm::vec3 &norm)
    {
        return vbo_.addVertex(Vertex{pos, norm, glm::vec2{}});
    }

    int addVertex(const glm::vec3 &pos)
    {
        return vbo_.addVertex(Vertex{pos, glm::vec3{}, glm::vec2{}});
    }

    glm::vec3 getVertexPos(int index) const { return vbo_.getVertex(index).pos; }
    void setVertexPos(int index, const glm::vec3 &pos) { vbo_.getVertex(index).pos = pos; }

    glm::vec3 getVertexNormal(int index) const { return vbo_.getVertex(index).norm; }
    void setVertexNormal(int index, const glm::vec3 &norm) { vbo_.getVertex(index).norm = norm; }

    glm::vec2 getVertexUV(int index) const { return vbo_.getVertex(index).uv; }
    void setVertexUV(int index, const glm::vec2 &uv) { vbo_.getVertex(index).uv = uv; }

    int getNumVertices() const { return vbo_.getNumVertices(); }

    // Indices
    int addIndex(unsigned int v1) { return ebo_.addIndex(v1); }

    void addIndices(unsigned int v1, unsigned int v2, unsigned int v3)
    {
        ebo_.addIndex(v1);
        ebo_.addIndex(v2);
        ebo_.addIndex(v3);
    }

    int getNumIndices() const { return ebo_.getNumIndices(); }

    void clearIndices() { ebo_.clear(); }

    // Mesh
    void clear()
    {
        vbo_.clear();
        ebo_.clear();
    }

    void flush(bool dynamic = false)
    {
        vbo_.flush(dynamic);
        ebo_.flush(dynamic);
    }

    void bind() { vao_.bind(); }

protected:
    struct Vertex
    {
        glm::vec3 pos{0.0f};
        glm::vec3 norm{0.0f};
        glm::vec2 uv{0.0f};
    };
    VertexBufferObject<Vertex> vbo_;
    VertexArrayObject vao_;
    IndexBufferObject ebo_;
};

class Visualizer
{
public:
    Visualizer()
    {
        lines_vao_.bind();
        lines_vao_.addAttributeFloat(3); // pos
        lines_vao_.addAttributeFloat(4); // color
        lines_vbo_.bind();
        lines_vao_.flush();

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
        lines_vbo_.addVertex(p);
        p.pos = s1;
        lines_vbo_.addVertex(p);
    }

    void addLine(const glm::vec3 &s0, const glm::vec3 &s1, const glm::vec4 &color0,
        const glm::vec4 &color1)
    {
        LinePoint p;
        p.color = color0;
        p.pos = s0;
        lines_vbo_.addVertex(p);
        p.color = color1;
        p.pos = s1;
        lines_vbo_.addVertex(p);
    }

    void render(const glm::mat4 &viewproj)
    {
        shader.bind();
        shader.setUniformMat4("uViewProj", viewproj);

        lines_vao_.bind();
        lines_vbo_.flush(true);
        glDrawArrays(GL_LINES, 0, lines_vbo_.getNumVertices());
        lines_vbo_.clear();
    }

private:
    struct LinePoint
    {
        glm::vec3 pos;
        glm::vec4 color;
    };
    Shader shader;
    VertexBufferObject<LinePoint> lines_vbo_;
    VertexArrayObject lines_vao_;
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
        glfwMaximizeWindow(window_);

        ///////////////////////////////////////////////////////////////////////////////
        Shader light_cube_shader("light_cube.shader");

        Mesh light_cube_mesh;
        light_cube_mesh.addVertex({0.0f, 1.0f, 0.0f});
        light_cube_mesh.addVertex({1.0f, 0.0f, 0.0f});
        light_cube_mesh.addVertex({-1.0f, 0.0f, 0.0f});
        light_cube_mesh.addVertex({0.0f, 0.0f, 1.0f});
        light_cube_mesh.addIndices(0, 1, 2);
        light_cube_mesh.addIndices(0, 1, 3);
        light_cube_mesh.addIndices(0, 2, 3);
        light_cube_mesh.addIndices(1, 3, 2);
        light_cube_mesh.flush();

        ///////////////////////////////////////////////////////////////////////////////
        Shader shader("shader.shader");

        ///////////////////////////////////////////////////////////////////////////////
        Image cat_image("image.png");
        Texture cat_texture(cat_image);

        glm::mat4 cat_transform = glm::mat4{1.0f};
        Mesh cat_mesh;
        {
            MeshLoader loader("object.obj");
            for (int i = 0; i < loader.getNumVertices(); i++)
            {
                cat_mesh.addVertex(loader.getVertexPosition(i), loader.getVertexNormal(i),
                    loader.getVertexTextureCoords(i));
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
        Mesh stickman_mesh;
        {
            MeshLoader loader("stickman.obj");
            for (int i = 0; i < loader.getNumVertices(); i++)
            {
                stickman_mesh.addVertex(loader.getVertexPosition(i), -loader.getVertexNormal(i),
                    loader.getVertexTextureCoords(i));
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

        Mesh floor_mesh;
        const float floor_size = 10.0f;
        const float floor_y = 0.0f;
        const float max_text_coord = 10.0f;
        floor_mesh.addVertex({-floor_size, floor_y, -floor_size}, {0, 1, 0}, {0.0f, 0.0f});
        floor_mesh.addVertex({-floor_size, floor_y, floor_size}, {0, 1, 0}, {0.0f, max_text_coord});
        floor_mesh.addVertex({floor_size, floor_y, floor_size}, {0, 1, 0},
            {max_text_coord, max_text_coord});
        floor_mesh.addVertex({floor_size, floor_y, -floor_size}, {0, 1, 0}, {max_text_coord, 0.0f});
        floor_mesh.addIndices(0, 1, 3);
        floor_mesh.addIndices(1, 2, 3);
        floor_mesh.flush();
        ////////////////////////////////////////////////

        camera_.setTransform(glm::translate(glm::mat4{1.0f}, glm::vec3(0.0f, 0.0f, 3.0f)));
        update_proj(window_);

        const auto visualize_normals = [](const Mesh &mesh, const glm::mat4 &transform) {
            return;
            const auto to_local = [&](const glm::vec3 &v) {
                return transform * glm::vec4(v, 1);
            };
            for (int i = 0; i < mesh.getNumVertices(); i++)
            {
                const auto pos = mesh.getVertexPos(i);
                const auto norm = mesh.getVertexNormal(i);
                engine_globals.visualizer->addLine(to_local(pos), to_local(pos + norm),
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

        while (!exit_)
        {
            shader.setDefine("USE_AMBIENT", use_ambient);
            shader.setDefine("USE_DIFFUSE", use_diffuse);
            shader.setDefine("USE_SPECULAR", use_specular);

            if (shader.isDirty())
            {
                shader.recompile();
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
            ImGui::Render();
            //////////////////////////////////////////////// IMGUI

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

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            anim_time += engine_globals.time->getDelta() * anim_time_multiplier;
            light_pos.x = sin(6 + anim_time * 1.0351) * 1.5f + 0.5f;
            light_pos.y = cos(1 + anim_time * 1.2561) * 1.5f + 1.3f;
            light_pos.z = sin(7 + anim_time * 1.125) * 1.5f + 0.5f;

            shader.bind();
            shader.setUniformVec3("uLight.color", light_color);
            shader.setUniformVec3("uLight.pos", light_pos);
            shader.setUniformMat4("uViewProj", camera_.getViewProj());
            if (use_ambient)
            {
                shader.setUniformFloat("uLight.ambientPower", ambient_power);
            }
            if (use_diffuse)
            {
                shader.setUniformFloat("uLight.diffusePower", diffuse_power);
            }
            if (use_specular)
            {
                shader.setUniformVec3("uCameraPos", camera_.getPosition());
                shader.setUniformFloat("uLight.specularPower", specular_power);
                shader.setUniformFloat("uMaterial.shininess", shininess);
            }

            glCullFace(GL_BACK);


            cat_transform = glm::rotate(glm::mat4{1.0f},
                                float(0.25 * engine_globals.time->getTime()),
                                glm::vec3(1.0f, 0.0f, 0.0f))
                * glm::scale(glm::mat4{1.0f}, glm::vec3{0.5f});
            shader.setUniformMat4("uModel", cat_transform);
            cat_texture.bind();
            cat_mesh.bind();
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glDrawElements(GL_TRIANGLES, cat_mesh.getNumIndices(), GL_UNSIGNED_INT, 0);


            ////////////////////////////////////////////////
            stickman_texture.bind();
            stickman_mesh.bind();
            stickman_mesh.flush();
            stickman_transform = glm::translate(glm::mat4{1.0f}, glm::vec3{1, 1, 0})
                * glm::scale(glm::mat4{1.0f}, glm::vec3{0.016f});
            shader.setUniformMat4("uModel", stickman_transform);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glDrawElements(GL_TRIANGLES, stickman_mesh.getNumIndices(), GL_UNSIGNED_INT, 0);

            ////////////////////////////////////////////////
            floor_texture.bind();
            floor_mesh.bind();
            floor_mesh.flush();
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
            light_cube_mesh.flush();
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDrawElements(GL_TRIANGLES, light_cube_mesh.getNumIndices(), GL_UNSIGNED_INT, 0);

            ///////////////////////////////////////////////
            visualize_normals(cat_mesh, cat_transform);
            visualize_normals(stickman_mesh, stickman_transform);
            visualize_normals(floor_mesh, glm::translate(glm::mat4{1.0f}, glm::vec3{0, -1, 0}));
            ////////////////////////////////////////////////
            engine_globals.visualizer->render(camera_.getViewProj());


            ////////////////// IMGUI
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            ////////////////// IMGUI

            glfwSwapBuffers(window_);

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
            engine_globals.engine_->framebuffer_size_callback(window, width, height);
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
