#pragma once

#include "UnicodeTable.h"

#include <imgui.h>

class MainWindow {
public:
    MainWindow() {
        uni_table::InitUtf8Table();
    }

    void Render() {

        ImGui::StyleColorsLight();
        {
            bool open = true;
            static bool use_work_area = true;
            static ImGuiWindowFlags flags =
                ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

            // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
            // Based on your use case you may want one or the other.
            const ImGuiViewport *viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
            ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

            if (ImGui::Begin(u8"例子例子Example: Fullscreen window", &open, flags)) {
                ImGui::Checkbox(u8"文件列表Use work area instead of main area", &use_work_area);
                ImGui::SameLine();

                ImGui::Text(u8"hello");
                ImGui::Text(u8"こんにちは"); // this will always be encoded as UTF-8
                ImGui::Text(
                    "こんにちは"); // the encoding of this is depending on compiler settings/flags and may be incorrect.

                ImGui::CheckboxFlags("ImGuiWindowFlags_NoBackground", &flags, ImGuiWindowFlags_NoBackground);
                ImGui::CheckboxFlags("ImGuiWindowFlags_NoDecoration", &flags, ImGuiWindowFlags_NoDecoration);
                ImGui::Indent();
                ImGui::CheckboxFlags("ImGuiWindowFlags_NoTitleBar", &flags, ImGuiWindowFlags_NoTitleBar);
                ImGui::CheckboxFlags("ImGuiWindowFlags_NoCollapse", &flags, ImGuiWindowFlags_NoCollapse);
                ImGui::CheckboxFlags("ImGuiWindowFlags_NoScrollbar", &flags, ImGuiWindowFlags_NoScrollbar);
                ImGui::Unindent();
            }
            ImGui::End();
        }

        {
            static uni_table::ExampleAssetsBrowser brower;
            bool open = true;
            brower.Draw("aa", &open);
        }

        // 在这里放置你所有的 UI 控件
        static bool show_demo_window = true;
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
    }
};