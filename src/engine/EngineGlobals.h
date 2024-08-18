#pragma once

class Engine;
class FileSystem;

struct EngineGlobals
{
    Engine *engine_{};
    FileSystem *fs{};
};

extern EngineGlobals engine_globals;