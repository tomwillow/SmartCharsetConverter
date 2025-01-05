#pragma once

#include "UnicodeTable.h"
#include "ListView.h"
#include "Core/Core.h"
#include "Translator/LanguageService.h"

#include <imgui.h>
#include <fmt/format.h>

class MainWindow {
public:
    MainWindow();

    void Render();

private:
    Core core;
    LanguageService languageService;
    ListView listView;
};