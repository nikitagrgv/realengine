#include "VoxelEngine.h"

// clang-format off
#include "glad/glad.h"
// clang-format on

#include "BlocksRegistry.h"
#include "EngineGlobals.h"
#include "TextureManager.h"

VoxelEngine::VoxelEngine() = default;

VoxelEngine::~VoxelEngine() = default;

void VoxelEngine::init()
{
    registry_ = makeU<BlocksRegistry>();

    Texture *atlas = eng.texture_manager->create("atlas");
    atlas->load("vox/atlas.png", Texture::Format::RGBA, Texture::Wrap::ClampToEdge,
        Texture::Filter::Nearest, Texture::Filter::Nearest, false);

    registry_->setAtlas(atlas, glm::ivec2(8, 8));
    register_blocks();
    registry_->flush();
}

void VoxelEngine::render()
{
    GL_CHECKED(glEnable(GL_BLEND));
    GL_CHECKED(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    GL_CHECKED(glEnable(GL_DEPTH_TEST));
    GL_CHECKED(glDepthMask(GL_TRUE));

    GL_CHECKED(glCullFace(GL_BACK));

    GL_CHECKED(glEnable(GL_CULL_FACE));


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