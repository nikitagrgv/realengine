#pragma once

#include "GlobalLight.h"
#include "Light.h"
#include "ShaderSource.h"
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
    void prerender(Camera *camera, Light *light);
    void renderWorld(Camera *camera, Light *light);

    void renderTexture2D(Texture *texture, glm::vec2 pos, glm::vec2 size);

    enum class TextPivot
    {
        TopLeft,
        BottomLeft
    };
    void renderText2D(const char *text, glm::vec2 pos, glm::vec2 viewport_size, float height_px,
        TextPivot pivot);

    Texture *getWhiteTexture() const { return base_.white_; }
    Texture *getBlackTexture() const { return base_.black_; }
    Texture *getNormalDefaulTexture() const { return base_.normal_default_; }

    Texture *getSkyboxTexture() const { return env_.skybox_; }
    void setSkyboxTexture(Texture *skybox) { env_.skybox_ = skybox; }

    GlobalLight getSunLight() const { return sun_light_; }
    void setSunLight(GlobalLight light) { sun_light_ = light; }

    void setEnvironmentMaterial(Material *material);
    Material *getEnvironmentMaterial() const;

private:
    void init_environment();
    void init_sprite();
    void init_text();

    void use_material(Material *material);

    void render_environment(Camera *camera);

private:
    struct
    {
        Texture *skybox_{};

        UPtr<ShaderSource> shader_source_;
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

        UPtr<ShaderSource> shader_source_;
        UPtr<Shader> shader_;

        int texture_loc_ = -1;
        int transform_loc_ = -1;
    } sprite_renderer_;

    struct TextRenderer
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

        UPtr<ShaderSource> shader_source_;
        UPtr<Shader> shader_;

        int texture_loc_ = -1;

        Texture *font_{};
    } text_renderer_;

    struct
    {
        Texture *white_{};
        Texture *black_{};
        Texture *normal_default_{};
    } base_;

    GlobalLight sun_light_{};
};
