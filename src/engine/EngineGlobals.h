#pragma once

class FileSystem;

struct EngineGlobals
{
    FileSystem *fs{};
};

extern EngineGlobals engine_globals;