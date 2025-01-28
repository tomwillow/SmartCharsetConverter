#include "MainWindow.h"
#include "resource.h"

#include <Common/tstring.h>
#include <Common/FileFunction.h>

// third party
#include <fmt/ranges.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <nlohmann/json.hpp>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <spdlog/spdlog.h>

#include <regex>

const std::string configFileName = "SmartCharsetConverter.json";

const std::vector<int> innerLanguageIds = {
    IDR_LANGUAGEJSON_ENGLISH,
    IDR_LANGUAGEJSON_SIMPLIFIED_CHINESE,
    IDR_LANGUAGEJSON_SPANISH,
};

EventAction PopupMessageBox(const std::string &text, const std::string &caption) noexcept {
    EventAction ret = EventAction::KEEP_ALIVE;
    ImGui::OpenPopup(caption.c_str());

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal(caption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text(text.c_str());
        ImGui::Separator();

        ImGui::SetItemDefaultFocus();
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            ret = EventAction::FINISH;
        }
        ImGui::EndPopup();
    }
    return ret;
}

MainWindow::MainWindow(GLFWwindow *glfwWindow)
    : glfwWindow(glfwWindow), core(configFileName, CoreInitOption{}),
      languageService([this]() -> LanguageServiceOption {
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

                bool changed = false;

                ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
                if (ImGui::TreeNode(
                        fmt::format(languageService.GetUtf8String(v0_2::StringId::SET_FILTER_MODE)).c_str())) {
                    changed |= ImGui::RadioButton(languageService.GetUtf8String(v0_2::StringId::NO_FILTER).c_str(),
                                                  reinterpret_cast<int *>(&core.GetConfigRef().filterMode),
                                                  static_cast<int>(Configuration::FilterMode::NO_FILTER));
                    changed |=
                        ImGui::RadioButton(languageService.GetUtf8String(v0_2::StringId::SMART_FILE_DETECTION).c_str(),
                                           reinterpret_cast<int *>(&core.GetConfigRef().filterMode),
                                           static_cast<int>(Configuration::FilterMode::SMART));
                    changed |=
                        ImGui::RadioButton(languageService.GetUtf8String(v0_2::StringId::USE_FILE_EXTENSION).c_str(),
                                           reinterpret_cast<int *>(&core.GetConfigRef().filterMode),
                                           static_cast<int>(Configuration::FilterMode::ONLY_SOME_EXTANT));
                    ImGui::BeginDisabled(core.GetConfigRef().filterMode != Configuration::FilterMode::ONLY_SOME_EXTANT);
                    ImGui::InputText("##", &core.GetConfigRef().includeRule);
                    ImGui::EndDisabled();

                    ImGui::TreePop();
                }

                ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
                if (ImGui::TreeNode(
                        fmt::format(languageService.GetUtf8String(v0_2::StringId::ADD_FILES_OR_FOLDER)).c_str())) {
                    ImGui::Button(languageService.GetUtf8String(v0_2::StringId::ADD_FILES).c_str());
                    ImGui::SameLine();
                    bool clicked = ImGui::Button(languageService.GetUtf8String(v0_2::StringId::ADD_FOLDER).c_str());
                    if (clicked) {
                        static std::wstring dir;

                        TFolderBrowser folderBrowser(glfwGetWin32Window(glfwWindow));
                        if (folderBrowser.Open(dir)) {
                            // AddItemsAsync(to_utf8(std::vector<std::wstring>{dir}));
                        }
                    }
                    ImGui::TreePop();
                }

                if (changed) {
                    core.WriteConfigToFile();
                }

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
        std::unique_lock<std::mutex> ul(guiEventsLock);
        localGuiEvents.insert(localGuiEvents.end(), guiEvents.begin(), guiEvents.end());
        guiEvents.clear();
    }

    for (auto it = localGuiEvents.begin(); it != localGuiEvents.end();) {
        auto &ev = *it;
        EventAction act = ev();
        if (act == EventAction::FINISH) {
            it = localGuiEvents.erase(it);
        } else {
            it++;
        }
    }
}

void MainWindow::PopupMessageBox(const std::string &text, const std::string &caption) noexcept {
    std::unique_lock<std::mutex> ul(guiEventsLock);
    guiEvents.push_back([this, text, caption]() -> EventAction {
        return ::PopupMessageBox(text, caption);
    });
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
                AddItems(filenames);
            });
        }

        ImGui::EndDragDropTarget();
    }
}

void MainWindow::CheckAndTraversalIncludeRule(std::function<void(const std::string &dotExt)> fn) {
    // 后缀字符串
    auto &extsStr = core.GetConfig().includeRule;

    // 切分
    auto exts = Split(extsStr, u8" ,|");

    std::string filterExampleStr = languageService.GetUtf8String(v0_2::StringId::SUPPORT_FORMAT_BELOW) +
                                   u8"\r\n *.h *.hpp *.c *.cpp *.txt\r\n h hpp c cpp txt\r\n h|hpp|c|cpp\r\n" +
                                   languageService.GetUtf8String(v0_2::StringId::SEPERATOR_DESCRIPTION);

    // 如果为空
    if (exts.empty()) {
        throw std::runtime_error(languageService.GetUtf8String(v0_2::StringId::NO_SPECIFY_FILTER_EXTEND) +
                                 u8"\r\n\r\n" + filterExampleStr);
    }

    // 逐个检查
    for (auto s : exts) {
        std::string extStr(s);
        std::string pattern = u8R"((\*\.|\.|)(\w+))"; // 匹配*.xxx/.xxx/xxx的正则
        std::regex r(pattern);
        std::smatch results;
        if (std::regex_match(extStr, results, r) == false) {
            throw std::runtime_error(languageService.GetUtf8String(v0_2::StringId::INVALID_EXTEND_FILTER) + extStr +
                                     u8"\r\n\r\n" + filterExampleStr);
        }

        fn(tolower(u8"." + results.str(2)));
    }
}

std::vector<std::string> MainWindow::AddItems(const std::vector<std::string> &pathes) noexcept {
    doCancel = false;
    // 后缀
    std::unordered_set<std::string> filterDotExts;

    switch (core.GetConfig().filterMode) {
    case Configuration::FilterMode::NO_FILTER:
        break;
    case Configuration::FilterMode::SMART: // 智能识别文本
        break;
    case Configuration::FilterMode::ONLY_SOME_EXTANT:
        // 只包括指定后缀
        try {
            CheckAndTraversalIncludeRule([&](const std::string &dotExt) {
                filterDotExts.insert(dotExt);
            });
        } catch (const std::runtime_error &err) {
            PopupMessageBox(err.what(), languageService.GetUtf8String(v0_2::StringId::MSGBOX_ERROR));
            return {};
        }
        break;
    default:
        assert(0);
    }

    std::vector<std::pair<std::string, std::string>> failed; // 失败的文件
    std::vector<std::string> ignored;                        // 忽略的文件

    auto AddItemNoException = [&](const std::string &filename) {
        try {
            Core::AddItemResult ret = core.AddItem(filename, filterDotExts);
            if (ret.isIgnore) {
                return;
            }
            listView.AddItem(
                ListView::MyItem{-1, filename, ret.filesize, ret.srcCharset, ret.srcLineBreak, to_utf8(ret.strPiece)});
        } catch (io_error_ignore) {
            ignored.push_back(filename);
        } catch (const MyRuntimeError &err) {
            failed.push_back({filename, err.ToLocalString(&languageService)});
        } catch (const std::runtime_error &err) {
            failed.push_back({filename, err.what()});
        }
    };

    for (auto &path : pathes) {
        // 如果是目录
        if (std::filesystem::is_directory(path)) {
            // 遍历指定目录
            auto filenames = TraversalAllFileNames(path);

            for (auto &filename : filenames) {
                if (doCancel) {
                    goto AddItemsAbort;
                }
                AddItemNoException(filename);
            }
            continue;
        }

        // 如果是文件
        if (doCancel) {
            goto AddItemsAbort;
        }
        AddItemNoException(path);
    }

AddItemsAbort:

    if (!failed.empty()) {
        std::string info = languageService.GetUtf8String(v0_2::StringId::FAILED_ADD_BELOW) + u8"\r\n";
        for (auto &pr : failed) {
            info += pr.first + u8" " + languageService.GetUtf8String(v0_2::StringId::REASON) + u8" " + pr.second +
                    u8"\r\n ";
        }

        PopupMessageBox(info, languageService.GetUtf8String(v0_2::StringId::MSGBOX_ERROR));
    }

    if (!ignored.empty()) {
        std::string s;

        std::string dest =
            fmt::format(languageService.GetUtf8String(v0_2::StringId::NON_TEXT_OR_NO_DETECTED), ignored.size());

        s += dest + u8"\r\n";

        int count = 0;
        for (auto &filename : ignored) {
            s += filename + u8"\r\n";
            count++;

            if (count >= 5) {
                s += languageService.GetUtf8String(v0_2::StringId::AND_SO_ON);
                break;
            }
        }

        s += u8"\r\n\r\n";
        s += languageService.GetUtf8String(v0_2::StringId::TIPS_USE_NO_FILTER);

        PopupMessageBox(s, languageService.GetUtf8String(v0_2::StringId::PROMPT));
        return ignored;
    }
    return ignored;
}