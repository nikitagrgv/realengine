#pragma once

#include "Statistics.h"


class Renderer;
class Time;
class Engine;
class FileSystem;
class Visualizer;
class MaterialManager;
class ShaderManager;
class TextureManager;
class MeshManager;
class World;
class Window;
class Input;
class SystemProxy;
class Gui;
class VoxelEngine;

struct EngineGlobals
{
    Engine *engine_{};
    FileSystem *fs{};
    Time *time{};
    Visualizer *visualizer{};
    MaterialManager *material_manager{};
    ShaderManager *shader_manager{};
    TextureManager *texture_manager{};
    MeshManager *mesh_manager{};
    World *world{};
    Renderer *renderer{};
    Window *window{};
    Input *input{};
    SystemProxy *proxy{};
    Gui *gui{};
    VoxelEngine *vox{};

    Statistics stat{};
};

extern EngineGlobals eng;