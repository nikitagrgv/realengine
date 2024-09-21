#pragma once

#include <Base.h>

namespace ImGui
{

namespace Utils
{

class ScopedId
{
public:
    explicit ScopedId(const char *str_id);
    explicit ScopedId(const void *ptr_id);
    explicit ScopedId(int int_id);
    ScopedId(const char *str_id_begin, const char *str_id_end);
    ~ScopedId();
};

} // namespace Utils

} // namespace ImGui


#define IMGUI_SCOPED_ID(...)                                                                       \
    ImGui::Utils::ScopedId REALENGINE_CONCATENATE(imgui_scoped_id_, __LINE__)(__VA_ARGS__)
