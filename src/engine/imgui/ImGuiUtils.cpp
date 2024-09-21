#include "ImguiUtils.h"

#include "imgui.h"

ImGui::Utils::ScopedId::ScopedId(const char *str_id)
{
    ImGui::PushID(str_id);
}

ImGui::Utils::ScopedId::ScopedId(const void *ptr_id)
{
    ImGui::PushID(ptr_id);
}

ImGui::Utils::ScopedId::ScopedId(int int_id)
{
    ImGui::PushID(int_id);
}

ImGui::Utils::ScopedId::ScopedId(const char *str_id_begin, const char *str_id_end)
{
    ImGui::PushID(str_id_begin, str_id_end);
}

ImGui::Utils::ScopedId::~ScopedId()
{
    ImGui::PopID();
}