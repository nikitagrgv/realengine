#pragma once

class Editor;

struct EditorGlobals
{
    Editor *editor_{};
};

extern EditorGlobals edg;