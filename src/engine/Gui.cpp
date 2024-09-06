#include "Gui.h"

// clang-format off
#include "EngineGlobals.h"
#include "Window.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
// clang-format on

Gui::Gui()
{
    GLFWwindow *window = eng.window->getHandle();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

Gui::~Gui() {}

void Gui::update()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    on_update_.call();

    ImGui::Render();
}


void Gui::swap()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}