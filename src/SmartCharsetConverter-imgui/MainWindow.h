#pragma once

#include "UnicodeTable.h"
#include "ListView.h"
#include "Core/Core.h"
#include "Translator/LanguageService.h"

// third party
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <fmt/format.h>
#include <BS_thread_pool.hpp>

enum class EventAction {
    KEEP_ALIVE,
    FINISH,
};

class MainWindow {
public:
    MainWindow(GLFWwindow *glfwWindow, const std::vector<std::string> &filenames = {});

    void Render();

private:
    // not own it, only ref it
    GLFWwindow *glfwWindow;
    Core core;
    LanguageService languageService;
    ListView listView;
    BS::thread_pool pool;

    mutable std::mutex guiEventsLock;
    std::vector<std::function<EventAction()>> guiEvents;
    std::vector<std::function<EventAction()>> localGuiEvents;

    std::wstring folderBrowserDir;

    std::atomic<bool> doCancel;

    void AddGuiEvent(std::function<EventAction()> guiEvent) noexcept;
    void HandleDragDrop();
    void CheckAndTraversalIncludeRule(std::function<void(const std::string &dotExt)> fn);
    void AddItems(const std::vector<std::string> &pathes) noexcept;
};