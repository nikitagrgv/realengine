#pragma once


class Time;
class Engine;
class FileSystem;
class Visualizer;
class MaterialManager;
class ShaderManager;
class TextureManager;
class MeshManager;

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
};

extern EngineGlobals EG;