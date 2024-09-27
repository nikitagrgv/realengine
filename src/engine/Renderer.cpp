#include "Renderer.h"

// clang-format off
#include <glad/glad.h>
// clang-format on

#include "Camera.h"
#include "EngineGlobals.h"
#include "Image.h"
#include "Light.h"
#include "Material.h"
#include "MaterialManager.h"
#include "Mesh.h"
#include "MeshManager.h"
#include "Shader.h"
#include "ShaderManager.h"
#include "Texture.h"
#include "TextureManager.h"
#include "Window.h"
#include "World.h"

#include <NodeMesh.h>

void Renderer::init()
{
    Image image;
    image.create(2, 2, Image::Format::RGB);

    base_.white_ = eng.texture_manager->create("white");
    image.fill(glm::u8vec4{255, 255, 255, 255});
    base_.white_->load(image);

    base_.black_ = eng.texture_manager->create("black");
    image.fill(glm::u8vec4{0, 0, 0, 255});
    base_.black_->load(image);

    base_.normal_default_ = eng.texture_manager->create("normal_default");
    image.fill(glm::u8vec4{128, 128, 255, 255});
    base_.normal_default_->load(image);

    init_environment();
    init_sprite();
}

void Renderer::clearBuffers()
{
    GL_CHECKED(glDepthMask(GL_TRUE));
    GL_CHECKED(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
    GL_CHECKED(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void Renderer::renderWorld(Camera *camera, Light *light)
{
    assert(camera);

    render_environment(camera);

    GL_CHECKED(glEnable(GL_BLEND));
    GL_CHECKED(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    GL_CHECKED(glEnable(GL_DEPTH_TEST));
    GL_CHECKED(glDepthMask(GL_TRUE));

    GL_CHECKED(glCullFace(GL_BACK));

    eng.window->bind();
    GL_CHECKED(glViewport(0, 0, eng.window->getWidth(), eng.window->getHeight()));

    for (int i = 0, count = eng.world->getNumNodes(); i < count; ++i)
    {
        Node *n = eng.world->getNodeByIndex(i);

        if (!n->isEnabled())
        {
            continue;
        }

        if (auto node = n->cast<NodeMesh>())
        {
            Mesh *mesh = node->getMesh();
            Material *mat = node->getMaterial();
            Shader *shader = mat->getShader();

            if (!mesh || !mat)
            {
                continue;
            }

            use_material(mat);
            mesh->bind();

            shader->setUniformMat4("uViewProj", camera->getViewProj());
            shader->setUniformVec3("uCameraPos", camera->getPosition());

            shader->setUniformMat4("uModel", node->getTransform());

            if (light)
            {
                shader->setUniformVec3("uLight.color", light->color);
                shader->setUniformVec3("uLight.pos", light->pos);
                shader->setUniformFloat("uLight.ambientPower", light->ambient_power);
                shader->setUniformFloat("uLight.diffusePower", light->diffuse_power);
                shader->setUniformFloat("uLight.specularPower", light->specular_power);
            }

            if (mat->isTwoSided())
            {
                GL_CHECKED(glDisable(GL_CULL_FACE));
            }
            else
            {
                GL_CHECKED(glEnable(GL_CULL_FACE));
            }

            GL_CHECKED(glDrawElements(GL_TRIANGLES, mesh->getNumIndices(), GL_UNSIGNED_INT, 0));
            eng.stat.addRenderedIndices(mesh->getNumIndices());
        }
    }
}

void Renderer::renderTexture(Texture *texture, glm::vec2 pos, glm::vec2 size)
{
    if (!texture)
    {
        return;
    }

    const SpriteRenderer &sr = sprite_renderer_;

    sr.shader_->bind();

    assert(sr.shader_->getUniformLocation("uTexture") == sr.texture_loc_);
    assert(sr.shader_->getUniformLocation("uTransform") == sr.transform_loc_);

    glm::mat3 mat;
    mat[0] = glm::vec3{2 * size.x, 0, 0};
    mat[1] = glm::vec3{0, -2 * size.x, 0};
    mat[2] = glm::vec3{-1 + 2 * pos.x, 1 - 2 * pos.y, 1};
    sr.shader_->setUniformMat3(sr.transform_loc_, mat);

    texture->bind(sr.texture_loc_);
    sr.vao_->bind();
    GL_CHECKED(glDisable(GL_CULL_FACE));
    GL_CHECKED(glDisable(GL_DEPTH_TEST));
    GL_CHECKED(glDrawArrays(GL_TRIANGLES, 0, sr.vbo_->getNumVertices()));
    eng.stat.addRenderedIndices(sr.vbo_->getNumVertices());
}

void Renderer::init_environment()
{
    env_.shader_source_ = makeU<ShaderSource>();
    env_.shader_source_->setFile("base/environment.shader");

    env_.material_ = eng.material_manager->create("environment");
    env_.material_->setShaderSource(env_.shader_source_.get());
    env_.material_->setTwoSided(true);
    env_.material_->addTexture("uSkybox");
    env_.material_->setTexture("uSkybox", getBlackTexture());

    // build cube
    env_.vao_ = makeU<VertexArrayObject>();
    env_.vbo_ = makeU<VertexBufferObject<glm::vec3>>();

    env_.vao_->bind();
    env_.vao_->addAttributeFloat(3); // pos
    env_.vbo_->bind();
    env_.vao_->flush();

    {
        // clang-format off
         constexpr float skybox_vertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };
        // clang-format on

        env_.vbo_->addRaw(skybox_vertices, sizeof(skybox_vertices));
        env_.vbo_->flush();
    }
}

void Renderer::init_sprite()
{
    SpriteRenderer &sr = sprite_renderer_;

    sr.vao_ = makeU<VertexArrayObject>();
    sr.vbo_ = makeU<VertexBufferObject<SpriteRenderer::Vertex>>();

    sr.vao_->bind();
    sr.vao_->addAttributeFloat(2);
    sr.vao_->addAttributeFloat(2);
    sr.vbo_->bind();
    sr.vao_->flush();

    {
        const SpriteRenderer::Vertex v0(glm::vec2{0, 0}, glm::vec2{0, 1});
        const SpriteRenderer::Vertex v1(glm::vec2{1, 0}, glm::vec2{1, 1});
        const SpriteRenderer::Vertex v2(glm::vec2{0, 1}, glm::vec2{0, 0});
        const SpriteRenderer::Vertex v3(glm::vec2{1, 1}, glm::vec2{1, 0});

        sr.vbo_->addVertex(v1);
        sr.vbo_->addVertex(v2);
        sr.vbo_->addVertex(v3);

        sr.vbo_->addVertex(v0);
        sr.vbo_->addVertex(v1);
        sr.vbo_->addVertex(v2);

        sr.vbo_->flush();
    }

    sr.shader_source_ = makeU<ShaderSource>();
    sr.shader_source_->setFile("base/sprite.shader");

    sr.shader_ = makeU<Shader>();
    sr.shader_->setSource(sr.shader_source_.get());
    sr.shader_->recompile();
    sr.texture_loc_ = sr.shader_->getUniformLocation("uTexture");
    sr.transform_loc_ = sr.shader_->getUniformLocation("uTransform");

    assert(sr.texture_loc_ != -1);
    assert(sr.transform_loc_ != -1);
}

void Renderer::use_material(Material *material)
{
    Shader *shader = material->getShader();

    if (shader->isDirty())
    {
        shader->recompile();
    }
    shader->bind();

    for (int i = 0, count = material->getNumTextures(); i < count; i++)
    {
        const int loc = shader->getUniformLocation(material->getTextureName(i).c_str());
        if (loc == -1)
        {
            continue;
        }
        material->getTexture(i)->bind(i);
        shader->setUniformInt(loc, i);
    }

    for (int i = 0, count = material->getNumParameters(); i < count; i++)
    {
        const int loc = shader->getUniformLocation(material->getParameterName(i).c_str());
        if (loc == -1)
        {
            continue;
        }
        const auto type = material->getParameterType(i);
        switch (type)
        {
        case Material::ParameterType::Float:
            shader->setUniformFloat(loc, material->getParameterFloat(i));
            break;
        case Material::ParameterType::Vec2:
            shader->setUniformVec2(loc, material->getParameterVec2(i));
            break;
        case Material::ParameterType::Vec3:
            shader->setUniformVec3(loc, material->getParameterVec3(i));
            break;
        case Material::ParameterType::Vec4:
            shader->setUniformVec4(loc, material->getParameterVec4(i));
            break;
        case Material::ParameterType::Mat4:
            shader->setUniformMat4(loc, material->getParameterMat4(i));
            break;
        default: break;
        }
    }
}

void Renderer::render_environment(Camera *camera)
{
    Shader *shader = env_.material_->getShader();
    if (shader->isDirty())
    {
        shader->recompile();
    }
    shader->bind();
    glm::mat4 viewproj = camera->getView();
    viewproj[3] = glm::vec4{0, 0, 0, 1};
    viewproj = camera->getProj() * viewproj;
    shader->setUniformMat4("uViewProj", viewproj);
    env_.vao_->bind();
    GL_CHECKED(glDisable(GL_CULL_FACE));
    GL_CHECKED(glDisable(GL_DEPTH_TEST));
    GL_CHECKED(glDepthMask(GL_FALSE));
    GL_CHECKED(glDrawArrays(GL_TRIANGLES, 0, env_.vbo_->getNumVertices()));
    eng.stat.addRenderedIndices(env_.vbo_->getNumVertices());
}