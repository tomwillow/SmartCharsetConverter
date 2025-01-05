#pragma once

#include "UnicodeTable.h"
#include "ListView.h"
#include <imgui.h>
#include <fmt/format.h>

class MainWindow {
public:
    MainWindow() {
        uni_table::InitUtf8Table();
    }

    void Render() {

        ImGui::StyleColorsLight();

        {
            ImGuiStyle &style = ImGui::GetStyle();
            style.FrameBorderSize = 1.0f;
            style.ScrollbarSize = 20.0f;
            style.ScrollbarRounding = 0.0f;
        }

        {
            bool open = true;
            static bool use_work_area = true;
            static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;

            // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
            // Based on your use case you may want one or the other.
            const ImGuiViewport *viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
            ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

            if (ImGui::Begin("Example: Simple layout", &open, flags)) {
                if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("File")) {
                        if (ImGui::MenuItem("Close", "Ctrl+W")) {
                            //*p_open = false;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }

                // Left
                static int selected = 0;
                {
                    ImGui::BeginGroup();
                    ImGui::BeginChild("left pane", ImVec2(150, 0), ImGuiChildFlags_ResizeX);
                    ImGui::SeparatorText(u8"文件列表");
                    {
                        ImGui::BeginChild("left pane", ImVec2(0, 0), ImGuiChildFlags_Border);
                        // for (int i = 0; i < 100; i++) {
                        //     // FIXME: Good candidate to use ImGuiSelectableFlags_SelectOnNav
                        //     char label[128];
                        //     sprintf(label, "MyObject %d", i);
                        //     if (ImGui::Selectable(label, selected == i))
                        //         selected = i;
                        // }
                        listView.Render();
                        ImGui::EndChild();
                    }
                    ImGui::EndChild();
                    ImGui::EndGroup();
                }
                ImGui::SameLine();

                // Right
                {
                    ImGui::BeginGroup();
                    ImGui::BeginChild("item view",
                                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
                    ImGui::Text("MyObject: %d", selected);
                    ImGui::Separator();
                    if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)) {
                        if (ImGui::BeginTabItem("Description")) {
                            ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do "
                                               "eiusmod tempor incididunt ut labore et dolore magna aliqua. ");
                            ImGui::EndTabItem();
                        }
                        if (ImGui::BeginTabItem("Details")) {
                            ImGui::Text("ID: 0123456789");
                            ImGui::EndTabItem();
                        }
                        ImGui::EndTabBar();
                    }
                    ImGui::EndChild();
                    if (ImGui::Button("Revert")) {}
                    ImGui::SameLine();
                    if (ImGui::Button("Save")) {}
                    ImGui::EndGroup();
                }
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

private:
    ListView listView;
};