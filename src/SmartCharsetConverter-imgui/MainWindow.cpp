#include "MainWindow.h"
#include "resource.h"

#include <Common/tstring.h>

#include <fmt/ranges.h>
#include <nlohmann/json.hpp>
#include <imgui_internal.h>
#include <spdlog/spdlog.h>

const std::string configFileName = "SmartCharsetConverter.json";

const std::vector<int> innerLanguageIds = {
    IDR_LANGUAGEJSON_ENGLISH,
    IDR_LANGUAGEJSON_SIMPLIFIED_CHINESE,
    IDR_LANGUAGEJSON_SPANISH,
};

MainWindow::MainWindow()
    : core(configFileName, CoreInitOption{}), languageService([this]() -> LanguageServiceOption {
          LanguageServiceOption option;
          option.languageName = core.GetConfig().language;
          option.resourceIds = innerLanguageIds;
          option.resourceType = L"LanguageJson";
          return option;
      }()),
      listView(languageService) {
    uni_table::InitUtf8Table();
}

void MainWindow::Render() {

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
                ImGui::SeparatorText(languageService.GetUtf8String(v0_2::StringId::FILE_LISTS).c_str());
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
            // here can catch the drop event at left panel
            HandleDragDrop();

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
            // here can catch the drop event at right panel
            HandleDragDrop();
        }
        ImGui::End();
        // in theory here is the whole window's end, but HandleDragDrop() not work if it's at here.
        // so have to put two HandleDragDrop at both left panel and right panel's end.
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

    {
        std::unique_lock ul(errMsgsLock);
        if (!errMsgs.empty()) {
            ImGui::OpenPopup(languageService.GetUtf8String(v0_2::StringId::MSGBOX_ERROR).c_str());
        }
    }

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal(languageService.GetUtf8String(v0_2::StringId::MSGBOX_ERROR).c_str(), NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text(languageService.GetUtf8String(v0_2::StringId::NON_TEXT_OR_NO_DETECTED).c_str());
        {
            std::unique_lock ul(errMsgsLock);
            for (auto &errMsg : errMsgs) {
                ImGui::Text(errMsg.c_str());
            }
        }
        ImGui::Separator();

        ImGui::SetItemDefaultFocus();
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            std::unique_lock ul(errMsgsLock);
            errMsgs.clear();
        }
        ImGui::EndPopup();
    }
}

void MainWindow::HandleDragDrop() {
    if (ImGui::BeginDragDropTarget()) {
        auto payload = ImGui::AcceptDragDropPayload("files", ImGuiDragDropFlags_AcceptBeforeDelivery);
        if (payload->IsDelivery()) {
            std::string s(reinterpret_cast<const char *>(payload->Data), payload->DataSize);
            auto j = nlohmann::json::parse(s);
            std::vector<std::string> filenames = j["data"].get<std::vector<std::string>>();
            fmt::print("accept: {}\n", filenames);
            pool.detach_task([this, filenames = std::move(filenames)]() {
                for (auto &filename : filenames) {
                    try {

                        auto ret = core.AddItem(filename, {});
                        listView.AddItem(ListView::MyItem{-1, filename, ret.filesize, ret.srcCharset, ret.srcLineBreak,
                                                          to_utf8(ret.strPiece)});
                    } catch (const std::runtime_error &err) {
                        fmt::print("{}", err.what());
                        std::unique_lock ul(errMsgsLock);
                        errMsgs.push_back(err.what());
                    }
                }
            });
        }

        ImGui::EndDragDropTarget();
    }
}
