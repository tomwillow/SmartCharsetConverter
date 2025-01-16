#pragma once

#include "UnicodeTable.h"
#include "ListView.h"
#include "Core/Core.h"
#include "Translator/LanguageService.h"

#include <imgui.h>
#include <fmt/format.h>
#include <BS_thread_pool.hpp>

class MainWindow {
public:
    MainWindow();

    void Render();

private:
    Core core;
    LanguageService languageService;
    ListView listView;
    BS::thread_pool pool;

    std::mutex errMsgsLock;
    std::vector<std::string> errMsgs;

    void HandleDragDrop();
};