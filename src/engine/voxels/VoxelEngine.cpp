#include "VoxelEngine.h"

// clang-format off
#include "glad/glad.h"
// clang-format on

#include "BlockInfo.h"
#include "BlocksRegistry.h"
#include "Camera.h"
#include "EngineGlobals.h"
#include "Shader.h"
#include "ShaderSource.h"
#include "TextureManager.h"
#include "VertexArrayObject.h"

VoxelEngine::VoxelEngine() = default;

VoxelEngine::~VoxelEngine() = default;

void VoxelEngine::init()
{
    registry_ = makeU<BlocksRegistry>();

    Texture *atlas = eng.texture_manager->create("atlas");
    atlas->load("vox/atlas.png", Texture::Format::RGBA, Texture::Wrap::ClampToEdge,
        Texture::Filter::Nearest, Texture::Filter::Nearest, Texture::FlipMode::FlipY);

    registry_->setAtlas(atlas, glm::ivec2(16, 16));
    register_blocks();
    registry_->flush();

    // shader
    shader_source_ = makeU<ShaderSource>();
    shader_source_->setFile("vox/vox.shader");

    shader_ = makeU<Shader>();
    shader_->setSource(shader_source_.get());
    shader_->recompile();

    // TODO#
    vao_ = makeU<VertexArrayObject>();
    vbo_ = makeU<VertexBufferObject<Vertex>>();

    vao_->bind();
    vao_->addAttributeFloat(3); // pos
    vao_->addAttributeFloat(2); // uv
    vbo_->bind();
    vao_->flush();
    vbo_->flush();

    generate_chunk();
}

void VoxelEngine::render(Camera *camera)
{
    GL_CHECKED(glEnable(GL_BLEND));
    GL_CHECKED(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    GL_CHECKED(glEnable(GL_DEPTH_TEST));
    GL_CHECKED(glDepthMask(GL_TRUE));

    GL_CHECKED(glCullFace(GL_BACK));

    GL_CHECKED(glEnable(GL_CULL_FACE));

    assert(!shader_->isDirty());
    shader_->bind();
    shader_->setUniformMat4("uViewProj", camera->getViewProj());

    // TODO# CACHE
    const int atlas_loc = shader_->getUniformLocation("atlas");
    assert(atlas_loc != -1);
    constexpr int atlas_index = 0;
    registry_->getAtlas()->bind(atlas_index);
    shader_->setUniformInt(atlas_loc, atlas_index);

    // TODO#
    vao_->bind();
    GL_CHECKED(glDrawArrays(GL_TRIANGLES, 0, vbo_->getNumVertices()));
    eng.stat.addRenderedIndices(vbo_->getNumVertices());
}

void VoxelEngine::register_blocks()
{
    BlocksRegistry &reg = *registry_;

    // Grass
    BlockDescription &grass = reg.addBlock();
    grass.name = "Grass";
    grass.type = BlockType::SOLID;
    grass.texture_index_px = 1;
    grass.texture_index_nx = 1;
    grass.texture_index_py = 0;
    grass.texture_index_ny = 2;
    grass.texture_index_pz = 1;
    grass.texture_index_nz = 1;
}

void VoxelEngine::generate_chunk()
{
    vbo_->clear();

    auto generate_block = [&](const BlockInfo &block) {
        const glm::vec3 min = block.position;
        const glm::vec3 max = min + glm::vec3(1, 1, 1);

        const BlockDescription &desc = registry_->getBlock(block.id);
        assert(desc.cached.valid);

        gen_face_py(min, max, desc);
        gen_face_ny(min, max, desc);
        gen_face_pz(min, max, desc);
        gen_face_nz(min, max, desc);
        gen_face_px(min, max, desc);
        gen_face_nx(min, max, desc);
    };

    BlockInfo block;
    block.id = 0;
    block.position = glm::ivec3(0, 0, 0);

    generate_block(block);

    vbo_->flush();
}

void VoxelEngine::gen_face_py(const glm::vec3 &min, const glm::vec3 &max, const BlockDescription &desc)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_py;
    Vertex v;

    // tr 1
    v.pos_ = glm::vec3{min.x, max.y, min.z};
    v.uv_ = coords.top_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{min.x, max.y, max.z};
    v.uv_ = coords.bottom_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, max.y, max.z};
    v.uv_ = coords.bottom_right;
    vbo_->addVertex(v);

    // tr 2
    v.pos_ = glm::vec3{min.x, max.y, min.z};
    v.uv_ = coords.top_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, max.y, max.z};
    v.uv_ = coords.bottom_right;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, max.y, min.z};
    v.uv_ = coords.top_right;
    vbo_->addVertex(v);
}

void VoxelEngine::gen_face_ny(const glm::vec3 &min, const glm::vec3 &max,
    const BlockDescription &desc)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_ny;
    Vertex v;

    // tr 1
    v.pos_ = glm::vec3{min.x, min.y, min.z};
    v.uv_ = coords.top_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, min.y, max.z};
    v.uv_ = coords.bottom_right;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{min.x, min.y, max.z};
    v.uv_ = coords.bottom_left;
    vbo_->addVertex(v);


    // tr 2
    v.pos_ = glm::vec3{min.x, min.y, min.z};
    v.uv_ = coords.top_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, min.y, min.z};
    v.uv_ = coords.top_right;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, min.y, max.z};
    v.uv_ = coords.bottom_right;
    vbo_->addVertex(v);
}

void VoxelEngine::gen_face_pz(const glm::vec3 &min, const glm::vec3 &max,
    const BlockDescription &desc)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_pz;
    Vertex v;

    // tr 1
    v.pos_ = glm::vec3{min.x, max.y, max.z};
    v.uv_ = coords.top_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{min.x, min.y, max.z};
    v.uv_ = coords.bottom_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, min.y, max.z};
    v.uv_ = coords.bottom_right;
    vbo_->addVertex(v);

    // tr 2
    v.pos_ = glm::vec3{min.x, max.y, max.z};
    v.uv_ = coords.top_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, min.y, max.z};
    v.uv_ = coords.bottom_right;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, max.y, max.z};
    v.uv_ = coords.top_right;
    vbo_->addVertex(v);
}

void VoxelEngine::gen_face_nz(const glm::vec3 &min, const glm::vec3 &max, const BlockDescription &desc)
{}

void VoxelEngine::gen_face_px(const glm::vec3 &min, const glm::vec3 &max, const BlockDescription &desc)
{}

void VoxelEngine::gen_face_nx(const glm::vec3 &min, const glm::vec3 &max,
    const BlockDescription &desc)
{}