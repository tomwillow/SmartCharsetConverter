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

class MainWindow {
public:
    MainWindow(GLFWwindow *glfwWindow);

    void Render();

private:
    // not own it, only ref it
    GLFWwindow *glfwWindow;
    Core core;
    LanguageService languageService;
    ListView listView;
    BS::thread_pool pool;

    std::mutex errMsgsLock;
    std::vector<std::string> errMsgs;

    void HandleDragDrop();
};