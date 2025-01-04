#include "FontLoader.h"

#include "FontAnalyzer.h"

#include <imgui.h>
#include <fmt/format.h>

#include <string>
#include <vector>
#include <filesystem>

struct FontInfo {
    std::string fontPath;
    int fontSize;
    std::vector<ImWchar> dict;
};

constexpr char windowsFontDir[] = "C:\\Windows\\Fonts\\";

void LoadFonts(ImFontAtlas *ioFonts) {

    std::vector<FontInfo> fontInfos{
        FontInfo{std::string(windowsFontDir) + "segoeui.ttf", 20, std::vector<ImWchar>{}},
        FontInfo{std::string(windowsFontDir) + "msyh.ttc", 20, std::vector<ImWchar>{}},
        FontInfo{std::string(windowsFontDir) + "simsun.ttc", 20, std::vector<ImWchar>{}},
    };

    ImFontConfig fontConfig;
    bool loadedTheFirst = false;

    for (std::size_t i = 0; i < fontInfos.size(); ++i) {
        if (loadedTheFirst) {
            fontConfig.MergeMode = true;
        }

        auto &fontInfo = fontInfos[i];
        if (!std::filesystem::is_regular_file(fontInfo.fontPath)) {
            continue;
        }

        FontAnalyzer<std::vector<ImWchar>> fontAnalyzer([](std::vector<ImWchar> &c, ImWchar val) {
            c.push_back(val);
        });
        auto ret = fontAnalyzer.GetUnicodePointRange(fontInfos[i].fontPath);
        ret.push_back(0);
        fontInfos[i].dict = std::move(ret);

        ImFont *font =
            ioFonts->AddFontFromFileTTF(fontInfos[i].fontPath.c_str(), static_cast<float>(fontInfos[i].fontSize),
                                        &fontConfig, fontInfos[i].dict.data());
        if (!font) {
            throw std::runtime_error(fmt::format("failed to add font: {}", fontInfo.fontPath));
        }
        loadedTheFirst = true;
    }

    ioFonts->Build();
    for (auto font : ioFonts->Fonts) {
        if (!font->IsLoaded()) {
            throw std::runtime_error(fmt::format("failed to load font: {}", font->ConfigData->Name));
        }
    }
}
