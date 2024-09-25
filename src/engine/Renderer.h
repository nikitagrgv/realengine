#pragma once

#include "VertexArrayObject.h"
#include "VertexBufferObject.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"


class Shader;
class Camera;
class Material;
class Light;
class Texture;
class ShaderSource;
class Mesh;

class Renderer
{
public:
    void init();

    void clearBuffers();
    void renderWorld(Camera *camera, Light *light);

    void renderTexture(Texture *texture, glm::vec2 pos, glm::vec2 size);

    Texture *getWhiteTexture() const { return base_.white_; }
    Texture *getBlackTexture() const { return base_.black_; }
    Texture *getNormalDefaulTexture() const { return base_.normal_default_; }

    Texture *getSkyboxTexture() const { return env_.skybox_; }
    void setSkyboxTexture(Texture *skybox) { env_.skybox_ = skybox; }

private:
    void init_environment();
    void init_sprite();

    void use_material(Material *material);

    void render_environment(Camera *camera);

private:
    struct
    {
        Texture *skybox_{};
        Material *material_{};

        UPtr<VertexBufferObject<glm::vec3>> vbo_;
        UPtr<VertexArrayObject> vao_;
    } env_;

    struct SpriteRenderer
    {
        struct Vertex
        {
            Vertex(const glm::vec2 &pos, const glm::vec2 &uv)
                : pos(pos)
                , uv(uv)
            {}
            glm::vec2 pos;
            glm::vec2 uv;
        };
        UPtr<VertexBufferObject<Vertex>> vbo_;
        UPtr<VertexArrayObject> vao_;
        UPtr<Shader> shader_;
        int texture_loc_ = -1;
        int matrix_loc_ = -1;
    } sprite_renderer_;

    struct
    {
        Texture *white_{};
        Texture *black_{};
        Texture *normal_default_{};
    } base_;
};
