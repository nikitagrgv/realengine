#include "Renderer.h"

// clang-format off
#include <glad/glad.h>
// clang-format on

#include "Camera.h"
#include "EngineGlobals.h"
#include "Light.h"
#include "Material.h"
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "World.h"

#include <NodeMesh.h>

void Renderer::clearBuffers()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::renderWorld(Camera *camera, Light *light)
{
    assert(camera);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);

    glCullFace(GL_BACK);

    eng.window->bind();
    glViewport(0, 0, eng.window->getWidth(), eng.window->getHeight());

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
                glDisable(GL_CULL_FACE);
            }
            else
            {
                glEnable(GL_CULL_FACE);
            }

            glDrawElements(GL_TRIANGLES, mesh->getNumIndices(), GL_UNSIGNED_INT, 0);
            num_rendered_indices_ += mesh->getNumIndices();
        }
    }
}

void Renderer::resetStatistics()
{
    num_rendered_indices_ = 0;
}

void Renderer::use_material(Material *material)
{
    Shader *shader = material->getShader();

    // MUST BE FIRST BECAUSE IT MAY BE RECOMPILED AFTER THIS
    shader->clearDefines();
    for (int i = 0, count = material->getNumDefines(); i < count; ++i)
    {
        if (material->getDefine(i))
        {
            shader->addDefine(material->getDefineName(i).c_str());
        }
    }

    shader->bind(); // recompile

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