#pragma once

#include "VertexArrayObject.h"
#include "VertexBufferObject.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"


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

    Texture *getSkyboxTexture() const { return env_.skybox; }
    void setSkyboxTexture(Texture *skybox) { env_.skybox = skybox; }

private:
    void init_environment();

    void use_material(Material *material);

    void render_environment(Camera *camera);

private:
    struct
    {
        Texture *skybox{};
        Material *material{};

        UPtr<VertexBufferObject<glm::vec3>> vbo_;
        UPtr<VertexArrayObject> vao_;
    } env_;

    struct
    {
        Texture *white_{};
        Texture *black_{};
        Texture *normal_default_{};
    } base_;
};
