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
#include <boost/thread/synchronized_value.hpp>

#include <regex>

const std::string configFileName = "SmartCharsetConverter.json";

const std::vector<int> innerLanguageIds = {
    IDR_LANGUAGEJSON_ENGLISH,
    IDR_LANGUAGEJSON_SIMPLIFIED_CHINESE,
    IDR_LANGUAGEJSON_SPANISH,
};

EventAction PopupMessageBox(const std::string &text, const std::string &caption,
                            std::function<EventAction()> fnInPopupScope = nullptr) noexcept {
    ImGui::OpenPopup(caption.c_str());

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal(caption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        std::shared_ptr<void> defer(nullptr, [](...) {
            ImGui::EndPopup();
        });

        ImGui::Text(text.c_str());
        ImGui::Separator();

        ImGui::SetItemDefaultFocus();
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            return EventAction::FINISH;
        }

        if (fnInPopupScope) {
            return fnInPopupScope();
        }
    }
    return EventAction::KEEP_ALIVE;
}

MainWindow::MainWindow(GLFWwindow *glfwWindow, const std::vector<std::string> &filenames)
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

    pool.detach_task([this, filenames = std::move(filenames)]() {
        AddItems(filenames);
    });
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

                // whether configuration is changed.
                // after all the flow, this flag indicates whether the configuration should be wrote to file.
                bool changed = false;

                // 1. selete filter mode
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

                // 2. add files or folder
                ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
                if (ImGui::TreeNode(
                        fmt::format(languageService.GetUtf8String(v0_2::StringId::ADD_FILES_OR_FOLDER)).c_str())) {
                    if (ImGui::Button(languageService.GetUtf8String(v0_2::StringId::ADD_FILES).c_str())) {
                        try {
                            std::vector<std::pair<std::wstring, std::wstring>> dialogFilter;
                            switch (core.GetConfig().filterMode) {
                            case Configuration::FilterMode::NO_FILTER:
                            case Configuration::FilterMode::SMART: // 智能识别文本
                                dialogFilter = {
                                    {languageService.GetWString(v0_2::StringId::ALL_FILES) + L"*.*", L"*.*"}};
                                break;
                            case Configuration::FilterMode::ONLY_SOME_EXTANT: {
                                // 只包括指定后缀
                                std::wstring filterExtsStr; // dialog的过滤器要求;分割
                                CheckAndTraversalIncludeRule([&](const std::string &dotExt) {
                                    filterExtsStr += utf8_to_wstring("*" + dotExt + ";");
                                });

                                // dialog过滤器
                                dialogFilter.push_back(make_pair(filterExtsStr, filterExtsStr));

                                break;
                            }
                            default:
                                assert(0);
                            }

                            // 打开文件对话框
                            TFileDialog dialog(glfwGetWin32Window(glfwWindow), dialogFilter, true);
                            if (dialog.Open()) {
                                auto filenames = to_utf8(dialog.GetResult());

                                pool.detach_task([this, filenames = std::move(filenames)]() {
                                    AddItems(filenames);
                                });
                            }
                        } catch (const std::runtime_error &err) {
                            PopupMessageBox(err.what(), languageService.GetUtf8String(v0_2::StringId::MSGBOX_ERROR));
                        }
                    }
                    ImGui::SameLine();
                    bool clicked = ImGui::Button(languageService.GetUtf8String(v0_2::StringId::ADD_FOLDER).c_str());
                    if (clicked) {
                        TFolderBrowser folderBrowser(glfwGetWin32Window(glfwWindow));
                        if (folderBrowser.Open(folderBrowserDir)) {
                            pool.detach_task([this]() {
                                AddItems({to_utf8(folderBrowserDir)});
                            });
                        }
                    }
                    ImGui::TreePop();
                }

                // 3. select output target
                ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
                if (ImGui::TreeNode(fmt::format(languageService.GetUtf8String(v0_2::StringId::SET_OUTPUT)).c_str())) {
                    changed |=
                        ImGui::RadioButton(languageService.GetUtf8String(v0_2::StringId::OUTPUT_TO_ORIGIN).c_str(),
                                           reinterpret_cast<int *>(&core.GetConfigRef().outputTarget),
                                           static_cast<int>(Configuration::OutputTarget::ORIGIN));
                    changed |=
                        ImGui::RadioButton(languageService.GetUtf8String(v0_2::StringId::OUTPUT_TO_FOLDER).c_str(),
                                           reinterpret_cast<int *>(&core.GetConfigRef().outputTarget),
                                           static_cast<int>(Configuration::OutputTarget::TO_DIR));

                    ImGui::BeginDisabled(core.GetConfigRef().outputTarget != Configuration::OutputTarget::TO_DIR);
                    ImGui::InputText("##", &core.GetConfigRef().outputDir);
                    ImGui::SameLine();
                    if (ImGui::Button(languageService.GetUtf8String(v0_2::StringId::SELECT_FOLDER).c_str())) {
                        TFolderBrowser folderBrowser(glfwGetWin32Window(glfwWindow));
                        if (folderBrowser.Open(folderBrowserDir)) {
                            core.SetOutputDir(to_utf8(folderBrowserDir));
                        }
                    }
                    ImGui::EndDisabled();

                    ImGui::TreePop();
                }

                // 4. specify output charset
                ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
                if (ImGui::TreeNode(
                        fmt::format(languageService.GetUtf8String(v0_2::StringId::SET_OUTPUT_CHARSET)).c_str())) {

                    // shown, outer charset (these charset are not in combo box)
                    const std::vector<CharsetCode> comboBoxOtherExcludes = {CharsetCode::UTF8, CharsetCode::UTF8BOM,
                                                                            CharsetCode::GB18030};
                    // FIXME: move to initializing
                    std::vector<CharsetCode> comboBoxOthers;
                    for (int icode = static_cast<int>(CharsetCode::UTF8);
                         icode < static_cast<int>(CharsetCode::CHARSET_CODE_END); ++icode) {
                        CharsetCode code = static_cast<CharsetCode>(icode);
                        if (std::find(comboBoxOtherExcludes.begin(), comboBoxOtherExcludes.end(), code) !=
                            comboBoxOtherExcludes.end()) {
                            continue;
                        }
                        comboBoxOthers.push_back(code);
                    }

                    for (auto charset : comboBoxOtherExcludes) {
                        changed |= ImGui::RadioButton(ToViewCharsetName(charset).c_str(),
                                                      reinterpret_cast<int *>(&core.GetConfigRef().outputCharset),
                                                      static_cast<int>(charset));
                    }

                    bool outputCharsetIsInOthersComboBox =
                        std::find(comboBoxOtherExcludes.begin(), comboBoxOtherExcludes.end(),
                                  core.GetConfigRef().outputCharset) == comboBoxOtherExcludes.end();
                    int selectedOthers = outputCharsetIsInOthersComboBox;
                    changed |= ImGui::RadioButton(languageService.GetUtf8String(v0_2::StringId::OTHERS).c_str(),
                                                  &selectedOthers, true);

                    CharsetCode comboSelectionCode;
                    if (outputCharsetIsInOthersComboBox) {
                        comboSelectionCode = core.GetConfigRef().outputCharset;
                    } else {
                        assert(comboBoxOthers.size() > 0);
                        comboSelectionCode = comboBoxOthers[0];
                    }

                    if (!outputCharsetIsInOthersComboBox && selectedOthers) {
                        core.GetConfigRef().outputCharset = comboSelectionCode;
                        changed = true;
                    }

                    ImGui::BeginDisabled(!outputCharsetIsInOthersComboBox);

                    ImGui::SameLine();
                    if (ImGui::BeginCombo("##combo 1", ToViewCharsetName(comboSelectionCode).c_str())) {
                        for (auto code : comboBoxOthers) {
                            if (ImGui::Selectable(ToViewCharsetName(code).c_str(),
                                                  core.GetConfigRef().outputCharset == code)) {
                                core.GetConfigRef().outputCharset = code;
                                changed = true;
                            }

                            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                            if (core.GetConfigRef().outputCharset == code) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }

                        ImGui::EndCombo();
                    }

                    ImGui::EndDisabled();

                    ImGui::TreePop();
                }

                if (changed) {
                    core.WriteConfigToFile();
                }

                ImGui::EndChild();
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

void MainWindow::AddGuiEvent(std::function<EventAction()> guiEvent) noexcept {
    std::unique_lock<std::mutex> ul(guiEventsLock);
    guiEvents.push_back(guiEvent);
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

void MainWindow::AddItems(const std::vector<std::string> &pathStrings) noexcept {
    std::shared_ptr<std::atomic<bool>> doCancel = std::make_shared<std::atomic<bool>>(false);

    // =========================================================
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
            AddGuiEvent([this, err]() -> EventAction {
                return ::PopupMessageBox(err.what(), languageService.GetUtf8String(v0_2::StringId::MSGBOX_ERROR));
            });
            return;
        }
        break;
    default:
        assert(0);
    }

    std::shared_ptr<std::atomic<bool>> finished = std::make_shared<std::atomic<bool>>(false);
    std::shared_ptr<void> defer = std::shared_ptr<void>(nullptr, [finished](...) {
        finished->store(true);
    });

    std::shared_ptr<boost::synchronized_value<std::string>> status =
        std::make_shared<boost::synchronized_value<std::string>>();
    std::shared_ptr<std::atomic<int>> progress = std::make_shared<std::atomic<int>>(-1);
    std::shared_ptr<std::atomic<int>> progressTotal = std::make_shared<std::atomic<int>>(-1);

    // =========================================================
    AddGuiEvent([this, doCancel, status, finished, progress, progressTotal]() -> EventAction {
        // FIXME
        ImGui::OpenPopup("adding");

        // Always center this window when appearing
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("adding", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            std::shared_ptr<void> defer(nullptr, [](...) {
                ImGui::EndPopup();
            });
            ImGui::Text(status->synchronize()->c_str());

            if (progress->load() < 0) {
                ImGui::ProgressBar(-1.0f * (float)ImGui::GetTime(), ImVec2(0.0f, 0.0f), "Searching.."); // FIXME
            } else {
                float fraction = static_cast<float>(progress->load()) / static_cast<float>(progressTotal->load());
                ImGui::ProgressBar(fraction, ImVec2(0.0f, 0.0f),
                                   fmt::format("{}/{}", progress->load(), progressTotal->load()).c_str()); // FIXME
            }
            ImGui::Separator();
            if (ImGui::Button(languageService.GetUtf8String(v0_2::StringId::CANCEL).c_str())) {
                doCancel->store(true);
            }
        }

        return finished->load() ? EventAction::FINISH : EventAction::KEEP_ALIVE;
    });

    // =========================================================
    status->synchronize()->assign(u8"正在统计文件数量..."); // FIXME
    std::vector<std::string> paths;
    for (auto &pathString : pathStrings) {
        if (doCancel->load()) {
            return;
        }
        auto path = std::filesystem::u8path(pathString);

        // 如果是目录
        if (std::filesystem::is_directory(path)) {
            for (auto p : std::filesystem::recursive_directory_iterator(path)) {
                paths.push_back(p.path().u8string());
                status->synchronize()->assign(fmt::format(u8"正在统计文件数量:{}...", paths.size())); // FIXME
            }
            continue;
        }

        // 如果是文件
        paths.push_back(path.u8string());
        status->synchronize()->assign(fmt::format(u8"正在统计文件数量:{}...", paths.size())); // FIXME
    }

    // =========================================================
    std::vector<std::pair<std::string, std::string>> failed; // 失败的文件
    std::vector<std::string> ignored;                        // 忽略的文件
    progressTotal->store(static_cast<int>(paths.size()));
    for (auto it = paths.begin(); it != paths.end(); it++) {
        if (doCancel->load()) {
            break;
        }
        progress->store(static_cast<int>(it - paths.begin()));
        auto &filename = *it;
        status->synchronize()->assign(u8"正在探测字符集..."); // FIXME

        try {
            Core::AddItemResult ret = core.AddItem(filename, filterDotExts);
            if (ret.isIgnore) {
                continue;
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
    }

    // =========================================================
    // show results
    std::shared_ptr<bool> closeButtonNotPressed = std::make_shared<bool>(true);
    AddGuiEvent([this, failed, ignored, closeButtonNotPressed]() -> EventAction {
        // 计算父窗口的 80% 宽高
        ImVec2 parentSize = ImGui::GetMainViewport()->WorkSize;

        // Always center this window when appearing
        // ImVec2 popupSize(parentSize.x * 0.8f, parentSize.y * 0.8f);
        // ImGui::SetNextWindowSize(popupSize, ImGuiCond_Appearing);
        ImVec2 maxPopupSize(parentSize.x * 0.8f, parentSize.y * 0.8f);

        // 设置弹出窗口的大小约束（允许缩小，但最大不超过 80%）
        ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), maxPopupSize);
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (failed.empty() && ignored.empty()) {
            return EventAction::FINISH;
        }

        if (!*closeButtonNotPressed) {
            return EventAction::FINISH;
        }
        // FIXME
        ImGui::OpenPopup("results");
        if (ImGui::BeginPopupModal("results", closeButtonNotPressed.get())) {
            std::shared_ptr<void> defer(nullptr, [](...) {
                ImGui::EndPopup();
            });

            ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
                                    ImGuiTableFlags_ScrollY; //  ImGuiTableFlags_NoBordersInBody |
            flags = ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;

            if (!failed.empty()) {
                ImGui::Text(languageService.GetUtf8String(v0_2::StringId::FAILED_ADD_BELOW).c_str());
                ImGui::Separator();
                if (ImGui::BeginTable("##failed add below", 3, flags)) {
                    ImGui::TableSetupColumn(u8"index", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn(u8"filename");
                    ImGui::TableSetupColumn(u8"reason");
                    ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
                    ImGui::TableHeadersRow();
                    int index = 1;
                    for (auto &pr : failed) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text(fmt::format("{}", index++).c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text(pr.first.c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text(pr.second.c_str());
                    }
                    ImGui::EndTable();
                }
            }
            if (!ignored.empty()) {
                ImGui::Text(
                    fmt::format(languageService.GetUtf8String(v0_2::StringId::NON_TEXT_OR_NO_DETECTED), ignored.size())
                        .c_str());
                ImGui::Separator();
                if (ImGui::BeginTable("##non text or no detected", 2, flags)) {
                    ImGui::TableSetupColumn(u8"index", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn(u8"filename");
                    ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
                    ImGui::TableHeadersRow();
                    int index = 1;

                    for (auto &filename : ignored) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text(fmt::format("{}", index++).c_str());
                        ImGui::TableNextColumn();
                        ImGui::Text(filename.c_str());
                    }
                    ImGui::EndTable();
                }

                ImGui::Text(languageService.GetUtf8String(v0_2::StringId::TIPS_USE_NO_FILTER).c_str());
            }

            ImGui::SetItemDefaultFocus();
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                return EventAction::FINISH;
            }
        }
        return EventAction::KEEP_ALIVE;
    });
    return;
}