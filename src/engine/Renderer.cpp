#include "Renderer.h"

// clang-format off
#include <glad/glad.h>
// clang-format on

#include "Camera.h"
#include "EngineGlobals.h"
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
    base_.white_ = eng.texture_manager->create("white");
    base_.white_->load("base/white.png");

    base_.black_ = eng.texture_manager->create("black");
    base_.black_->load("base/black.png");

    base_.normal_default_ = eng.texture_manager->create("normal_default");
    base_.normal_default_->load("base/normal_default.png");

    init_environment();
}

void Renderer::clearBuffers()
{
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

void Renderer::init_environment()
{
    ShaderSource *cubemap_shader = eng.shader_manager->create("environment");
    cubemap_shader->setFile("base/environment.shader");

    env_.material = eng.material_manager->create("environment");
    env_.material->setShaderSource(cubemap_shader);
    env_.material->setTwoSided(true);
    env_.material->addTexture("uSkybox");
    env_.material->setTexture("uSkybox", getBlackTexture());

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
    Shader *shader = env_.material->getShader();
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
}