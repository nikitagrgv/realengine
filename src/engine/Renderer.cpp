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
#include "voxels/VoxelEngine.h"

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
    init_text();
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

    eng.window->bind();
    GL_CHECKED(glViewport(0, 0, eng.window->getWidth(), eng.window->getHeight()));

    render_environment(camera);

    eng.vox->render(camera, &global_light_);

    GL_CHECKED(glEnable(GL_BLEND));
    GL_CHECKED(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    GL_CHECKED(glEnable(GL_DEPTH_TEST));
    GL_CHECKED(glDepthMask(GL_TRUE));

    GL_CHECKED(glCullFace(GL_BACK));

    for (int i = 0, count = eng.world->getNumNodes(); i < count; ++i)
    {
        Node *n = eng.world->getNodeByIndex(i);

        if (!n->isEnabled())
        {
            continue;
        }

        if (auto node = n->cast<NodeMesh>())
        {
            const Mesh *mesh = node->getMesh();
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

void Renderer::renderTexture2D(Texture *texture, glm::vec2 pos, glm::vec2 size)
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
    mat[1] = glm::vec3{0, -2 * size.y, 0};
    mat[2] = glm::vec3{-1 + 2 * pos.x, 1 - 2 * pos.y, 1};
    sr.shader_->setUniformMat3(sr.transform_loc_, mat);

    texture->bind(sr.texture_loc_);
    sr.vao_->bind();
    GL_CHECKED(glDisable(GL_CULL_FACE));
    GL_CHECKED(glDisable(GL_DEPTH_TEST));
    GL_CHECKED(glDrawArrays(GL_TRIANGLES, 0, sr.vbo_->getNumVertices()));
    eng.stat.addRenderedIndices(sr.vbo_->getNumVertices());
}

void Renderer::renderText2D(const char *text, glm::vec2 pos, glm::vec2 viewport_size,
    float height_px, TextPivot pivot)
{
    TextRenderer &tr = text_renderer_;

    VertexBufferObject<TextRenderer::Vertex> &vbo = *tr.vbo_;

    constexpr glm::vec2 step_uv_pixels_size{20.0f, 32.0f};
    constexpr int char_margin_pixels_top{4};
    constexpr int char_margin_pixels_bottom{3};

    const glm::vec2 atlas_size = glm::ivec2(tr.font_->getSize());

    const glm::vec2 char_margin_top_left{0, char_margin_pixels_top / atlas_size.y};
    const glm::vec2 char_margin_bottom_right{0, char_margin_pixels_bottom / atlas_size.y};
    const glm::vec2 char_margin_total = char_margin_top_left + char_margin_bottom_right;

    const glm::vec2 step_uv_size = step_uv_pixels_size / atlas_size;
    const glm::vec2 char_uv_size = step_uv_size - char_margin_total;

    const float width_px = height_px * char_uv_size.x / char_uv_size.y;
    const float height = height_px / viewport_size.y;
    const float width = width_px / viewport_size.x;

    constexpr int chars_in_row = 12;

    vbo.clear();

    glm::vec2 cur_pos = pos;
    switch (pivot)
    {
    case TextPivot::TopLeft:
    {
        break;
    }
    case TextPivot::BottomLeft:
    {
        cur_pos.y -= height;
        break;
    }
    }

    const auto calc_uv = [&](char ch, glm::vec2 &top_left, glm::vec2 &bottom_right) {
        assert(ch >= 0x20 && ch < 0x80);
        const int glob_offset = ch - 0x20;
        const int row = glob_offset / chars_in_row;
        const int column = glob_offset % chars_in_row;

        top_left.x = step_uv_size.x * static_cast<float>(column);
        top_left.y = step_uv_size.y * static_cast<float>(row);
        top_left += char_margin_top_left;

        bottom_right = top_left + char_uv_size;
    };

    glm::mat3 mat;
    mat[0] = glm::vec3{2, 0, 0};
    mat[1] = glm::vec3{0, -2, 0};
    mat[2] = glm::vec3{-1, 1, 1};

    const char *p = text;
    while (*p)
    {
        const char ch = *p;

        if (ch >= 0x20 && ch < 0x80)
        {
            glm::vec2 top_left_uv;
            glm::vec2 bot_right_uv;
            calc_uv(ch, top_left_uv, bot_right_uv);

            const glm::vec2 top_left_pos = mat * glm::vec3{cur_pos.x, cur_pos.y, 1};
            const glm::vec2 bot_right_pos = top_left_pos
                + glm::vec2{
                    mat * glm::vec3{width, height, 0}
            };

            vbo.addVertex(TextRenderer::Vertex{top_left_pos, top_left_uv});
            vbo.addVertex(TextRenderer::Vertex{glm::vec2(top_left_pos.x, bot_right_pos.y),
                glm::vec2(top_left_uv.x, bot_right_uv.y)});
            vbo.addVertex(TextRenderer::Vertex{glm::vec2(bot_right_pos.x, top_left_pos.y),
                glm::vec2(bot_right_uv.x, top_left_uv.y)});

            vbo.addVertex(TextRenderer::Vertex{glm::vec2(top_left_pos.x, bot_right_pos.y),
                glm::vec2(top_left_uv.x, bot_right_uv.y)});
            vbo.addVertex(TextRenderer::Vertex{glm::vec2(bot_right_pos.x, top_left_pos.y),
                glm::vec2(bot_right_uv.x, top_left_uv.y)});
            vbo.addVertex(TextRenderer::Vertex{bot_right_pos, bot_right_uv});
        }

        cur_pos.x += width;
        ++p;
    }

    if (vbo.getNumVertices() == 0)
    {
        return;
    }

    vbo.flush(true);

    tr.shader_->bind();

    assert(tr.shader_->getUniformLocation("uTexture") == tr.texture_loc_);

    tr.font_->bind(tr.texture_loc_);
    tr.vao_->bind();
    GL_CHECKED(glDisable(GL_CULL_FACE));
    GL_CHECKED(glDisable(GL_DEPTH_TEST));
    GL_CHECKED(glDrawArrays(GL_TRIANGLES, 0, tr.vbo_->getNumVertices()));
    eng.stat.addRenderedIndices(tr.vbo_->getNumVertices());
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

void Renderer::init_text()
{
    TextRenderer &tr = text_renderer_;

    tr.font_ = eng.texture_manager->create("default_font");
    tr.font_->load("base/default_font_20x32.png", Texture::Format::RGBA, Texture::Wrap::ClampToEdge,
        Texture::Filter::Nearest, Texture::Filter::Nearest, Texture::FlipMode::DontFlip);

    tr.vao_ = makeU<VertexArrayObject>();
    tr.vbo_ = makeU<VertexBufferObject<TextRenderer::Vertex>>();

    tr.vao_->bind();
    tr.vao_->addAttributeFloat(2);
    tr.vao_->addAttributeFloat(2);
    tr.vbo_->bind();
    tr.vao_->flush();

    tr.vbo_->flush();

    tr.shader_source_ = makeU<ShaderSource>();
    tr.shader_source_->setFile("base/text.shader");

    tr.shader_ = makeU<Shader>();
    tr.shader_->setSource(tr.shader_source_.get());
    tr.shader_->recompile();
    tr.texture_loc_ = tr.shader_->getUniformLocation("uTexture");

    assert(tr.texture_loc_ != -1);
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